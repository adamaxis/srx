#include "world_content_tsv_loader.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <limits>
#include <map>
#include <set>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

extern "C" {
#include "lauxlib.h"
}

namespace shadowrun::ghidra_conversion::scripting::detail {
namespace {

struct TsvDocument {
    std::vector<std::string> columns;
    std::vector<std::vector<std::string>> rows;
    std::map<std::string, std::size_t, std::less<>> column_indexes;
};

bool parse_tsv(std::string_view source, TsvDocument& document,
               std::string& failure) {
    std::vector<std::vector<std::string>> records;
    std::vector<std::string> record;
    std::string field;
    bool quoted = false;
    bool field_started = false;

    const auto finish_field = [&] {
        record.push_back(std::move(field));
        field.clear();
        field_started = false;
    };
    const auto finish_record = [&] {
        finish_field();
        const bool empty = std::all_of(
            record.begin(), record.end(),
            [](const std::string& value) { return value.empty(); });
        if (!empty) {
            records.push_back(std::move(record));
        }
        record.clear();
    };

    for (std::size_t index = 0; index < source.size(); ++index) {
        const char value = source[index];
        if (quoted) {
            if (value == '"') {
                if (index + 1U < source.size() &&
                    source[index + 1U] == '"') {
                    field.push_back('"');
                    ++index;
                } else {
                    quoted = false;
                }
            } else {
                field.push_back(value);
            }
            continue;
        }
        if (value == '"' && !field_started && field.empty()) {
            quoted = true;
            field_started = true;
        } else if (value == '\t') {
            finish_field();
        } else if (value == '\n' || value == '\r') {
            finish_record();
            if (value == '\r' && index + 1U < source.size() &&
                source[index + 1U] == '\n') {
                ++index;
            }
        } else {
            field.push_back(value);
            field_started = true;
        }
    }
    if (quoted) {
        failure = "TSV contains an unterminated quoted field";
        return false;
    }
    if (!field.empty() || !record.empty() || field_started) {
        finish_record();
    }
    if (records.empty()) {
        return true;
    }

    document.columns = std::move(records.front());
    if (!document.columns.empty() && document.columns.front().size() >= 3U &&
        document.columns.front().compare(0U, 3U, "\xEF\xBB\xBF") == 0) {
        document.columns.front().erase(0U, 3U);
    }
    for (std::size_t column = 0; column < document.columns.size(); ++column) {
        if (document.columns[column].empty() ||
            !document.column_indexes
                 .emplace(document.columns[column], column)
                 .second) {
            failure = "TSV has an empty or duplicate column name";
            return false;
        }
    }
    for (std::size_t row_index = 1; row_index < records.size(); ++row_index) {
        if (records[row_index].size() != document.columns.size()) {
            failure = "TSV row " + std::to_string(row_index + 1U) +
                      " has a different number of fields than its header";
            return false;
        }
        document.rows.push_back(std::move(records[row_index]));
    }
    return true;
}

bool read_tsv(const std::filesystem::path& path, const bool optional,
              TsvDocument& document, std::string& failure) {
    std::error_code error;
    const bool exists = std::filesystem::exists(path, error);
    if (error == std::errc::no_such_file_or_directory) {
        error.clear();
        if (optional) {
            document = {};
            return true;
        }
    }
    if (error) {
        failure = "could not inspect TSV " + path.string() + ": " +
                  error.message();
        return false;
    }
    if (!exists) {
        if (optional) {
            document = {};
            return true;
        }
        failure = "required world-content TSV is missing: " + path.string();
        return false;
    }
    if (!std::filesystem::is_regular_file(path, error) || error) {
        failure = "world-content TSV is not a regular file: " +
                  path.string();
        if (error) {
            failure += ": " + error.message();
        }
        return false;
    }
    std::ifstream stream(path, std::ios::binary);
    if (!stream) {
        failure = "could not open world-content TSV: " + path.string();
        return false;
    }
    const std::string source{
        std::istreambuf_iterator<char>(stream),
        std::istreambuf_iterator<char>()};
    if (!stream.eof() && stream.fail()) {
        failure = "could not read world-content TSV: " + path.string();
        return false;
    }
    if (!parse_tsv(source, document, failure)) {
        failure = path.string() + ": " + failure;
        return false;
    }
    return true;
}

std::string_view value_at(const TsvDocument& document,
                          const std::vector<std::string>& row,
                          const std::string_view column) {
    const auto found = document.column_indexes.find(column);
    if (found == document.column_indexes.end() ||
        found->second >= row.size()) {
        return {};
    }
    return row[found->second];
}

bool parse_integer(const std::string_view text, lua_Integer& result) {
    if (text.empty() || text == "-") {
        result = 0;
        return true;
    }
    bool negative = false;
    std::string_view digits = text;
    if (digits.front() == '-') {
        negative = true;
        digits.remove_prefix(1U);
    }
    int base = 10;
    if (digits.size() > 2U && digits[0] == '0' &&
        (digits[1] == 'x' || digits[1] == 'X')) {
        base = 16;
        digits.remove_prefix(2U);
    }
    if (digits.empty()) {
        return false;
    }
    std::uint64_t magnitude = 0U;
    const auto parsed = std::from_chars(
        digits.data(), digits.data() + digits.size(), magnitude, base);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != digits.data() + digits.size()) {
        return false;
    }
    const auto maximum = static_cast<std::uint64_t>(
        std::numeric_limits<lua_Integer>::max());
    if ((!negative && magnitude > maximum) ||
        (negative && magnitude > maximum + 1U)) {
        return false;
    }
    if (negative && magnitude == maximum + 1U) {
        result = std::numeric_limits<lua_Integer>::min();
    } else {
        const auto signed_magnitude = static_cast<lua_Integer>(magnitude);
        result = negative ? -signed_magnitude : signed_magnitude;
    }
    return true;
}

bool numeric_column(const std::string_view name,
                    const std::vector<std::string_view>& numeric_columns) {
    return std::find(numeric_columns.begin(), numeric_columns.end(), name) !=
           numeric_columns.end();
}

std::string_view lua_column_name(
    const std::string_view tsv_column,
    const std::vector<std::pair<std::string_view, std::string_view>>& aliases) {
    for (const auto& [source, destination] : aliases) {
        if (source == tsv_column) {
            return destination;
        }
    }
    return tsv_column;
}

bool push_record(lua_State* state, const TsvDocument& document,
                 const std::vector<std::string>& row,
                 const std::vector<std::string_view>& numeric_columns,
                 const std::vector<
                     std::pair<std::string_view, std::string_view>>& aliases,
                 const std::filesystem::path& source_path,
                 const std::size_t row_number, std::string& failure) {
    lua_createtable(state, 0, static_cast<int>(document.columns.size()));
    const auto table_index = lua_absindex(state, -1);
    for (const auto& column : document.columns) {
        const auto value = value_at(document, row, column);
        if (numeric_column(column, numeric_columns)) {
            lua_Integer number = 0;
            if (!parse_integer(value, number)) {
                failure = source_path.string() + ": row " +
                          std::to_string(row_number) + " column " + column +
                          " is not an integer";
                lua_pop(state, 1);
                return false;
            }
            lua_pushinteger(state, number);
        } else {
            lua_pushlstring(state, value.data(), value.size());
        }
        const auto destination = lua_column_name(column, aliases);
        lua_setfield(state, table_index, std::string(destination).c_str());
    }
    return true;
}

bool push_rows(lua_State* state, const TsvDocument& document,
               const std::vector<std::string_view>& numeric_columns,
               const std::filesystem::path& source_path,
               std::string& failure,
               const std::vector<
                   std::pair<std::string_view, std::string_view>>& aliases = {}) {
    lua_createtable(state, static_cast<int>(document.rows.size()), 0);
    const auto array_index = lua_absindex(state, -1);
    for (std::size_t index = 0; index < document.rows.size(); ++index) {
        if (!push_record(state, document, document.rows[index],
                         numeric_columns, aliases, source_path, index + 2U,
                         failure)) {
            lua_pop(state, 1);
            return false;
        }
        lua_rawseti(state, array_index, static_cast<lua_Integer>(index + 1U));
    }
    return true;
}

bool set_text_field(lua_State* state, const int table_index,
                    const char* field_name, const std::string_view value,
                    const bool optional = false) {
    const auto normalized =
        optional && value == "-" ? std::string_view{} : value;
    lua_pushlstring(state, normalized.data(), normalized.size());
    lua_setfield(state, table_index, field_name);
    return true;
}

bool push_entity(lua_State* state, const std::filesystem::path& directory,
                 const std::string& entity_id, std::string& failure) {
    const auto entity_path = directory / "entity.tsv";
    TsvDocument entity_document;
    if (!read_tsv(entity_path, true, entity_document, failure)) {
        return false;
    }
    if (entity_document.rows.empty()) {
        lua_pushnil(state);
        return true;
    }

    const auto& metadata = entity_document.rows.front();
    const auto metadata_index = [&](const std::string_view column) {
        return value_at(entity_document, metadata, column);
    };
    lua_createtable(state, 0, 10);
    const auto entity_table = lua_absindex(state, -1);
    set_text_field(state, entity_table, "id",
                   metadata_index("entity_id").empty()
                       ? std::string_view(entity_id)
                       : metadata_index("entity_id"));
    set_text_field(state, entity_table, "name", metadata_index("name"));
    set_text_field(state, entity_table, "category",
                   metadata_index("category"));
    set_text_field(state, entity_table, "evidence",
                   metadata_index("evidence"));
    set_text_field(state, entity_table, "content_profile",
                   metadata_index("content_profile"), true);
    set_text_field(state, entity_table, "speaker_id",
                   metadata_index("speaker_id"), true);
    set_text_field(state, entity_table, "shop_profile",
                   metadata_index("shop_profile"), true);

    struct ChildFile {
        const char* filename;
        const char* key;
        std::vector<std::string_view> numeric_columns;
        std::vector<std::pair<std::string_view, std::string_view>> aliases;
    };
    const std::vector<ChildFile> children{
        {"parts.tsv", "parts",
         {"source_scene", "source_x", "source_y", "width", "height",
          "local_x", "local_y"},
         {{"part_id", "id"}}},
        {"interactables.tsv", "interactables",
         {"local_x", "local_y", "width", "height"},
         {{"interaction_id", "id"}}},
        {"collisions.tsv", "collision_regions",
         {"local_x", "local_y", "width", "height"},
         {{"collision_id", "id"}}},
    };
    for (const auto& child : children) {
        const auto child_path = directory / child.filename;
        TsvDocument document;
        if (!read_tsv(child_path, true, document, failure) ||
            !push_rows(state, document, child.numeric_columns, child_path,
                       failure, child.aliases)) {
            lua_pop(state, 1);
            return false;
        }
        lua_setfield(state, entity_table, child.key);
    }
    return true;
}

bool push_entities(lua_State* state, const std::filesystem::path& world_root,
                   std::string& failure) {
    const auto entity_root = world_root / "entities";
    std::error_code error;
    if (!std::filesystem::is_directory(entity_root, error)) {
        if (!error) {
            failure = "world entity directory is missing: " +
                      entity_root.string();
        } else {
            failure = "could not inspect world entity directory: " +
                      error.message();
        }
        return false;
    }
    std::vector<std::filesystem::path> directories;
    for (std::filesystem::directory_iterator iterator(entity_root, error), end;
         !error && iterator != end; iterator.increment(error)) {
        if (iterator->is_directory(error)) {
            directories.push_back(iterator->path());
        }
    }
    if (error) {
        failure = "could not enumerate world entities: " + error.message();
        return false;
    }
    std::sort(directories.begin(), directories.end());

    lua_createtable(state, 0, static_cast<int>(directories.size()));
    const auto entities_index = lua_absindex(state, -1);
    std::set<std::string> identifiers;
    for (const auto& directory : directories) {
        const auto entity_id = directory.filename().string();
        if (!push_entity(state, directory, entity_id, failure)) {
            lua_pop(state, 1);
            return false;
        }
        if (lua_isnil(state, -1)) {
            lua_pop(state, 1);
            continue;
        }
        const auto entity_table = lua_absindex(state, -1);
        lua_getfield(state, entity_table, "id");
        std::size_t id_length = 0U;
        const auto* id_value = lua_tolstring(state, -1, &id_length);
        if (id_value == nullptr || id_length == 0U) {
            lua_pop(state, 2);
            lua_pop(state, 1);
            failure = "entity definition has an empty entity_id: " +
                      directory.string();
            return false;
        }
        const std::string key(id_value, id_length);
        lua_pop(state, 1);
        if (!identifiers.insert(key).second) {
            lua_pop(state, 1);
            lua_pop(state, 1);
            failure = "duplicate world entity ID: " + key;
            return false;
        }
        lua_setfield(state, entities_index, key.c_str());
    }
    return true;
}

bool scene_file(const std::filesystem::path& path, int& scene_id,
                std::string& suffix) {
    const auto filename = path.filename().string();
    if (filename.size() < 10U || filename.rfind("scene-", 0U) != 0U ||
        path.extension() != ".tsv") {
        return false;
    }
    const auto stem = path.stem().string();
    const auto delimiter = stem.find('-', 6U);
    const auto digits = std::string_view(stem).substr(
        6U, delimiter == std::string::npos ? std::string::npos
                                            : delimiter - 6U);
    if (digits.empty() ||
        !std::all_of(digits.begin(), digits.end(),
                     [](const char value) {
                         return value >= '0' && value <= '9';
                     })) {
        return false;
    }
    const auto parsed =
        std::from_chars(digits.data(), digits.data() + digits.size(), scene_id);
    if (parsed.ec != std::errc{} ||
        parsed.ptr != digits.data() + digits.size()) {
        return false;
    }
    if (delimiter == std::string::npos) {
        suffix.clear();
        return true;
    }
    suffix = stem.substr(delimiter + 1U);
    return suffix == "triggers" || suffix == "spawns" ||
           suffix == "encounters";
}

std::string row_string(const TsvDocument& document,
                       const std::vector<std::string>& row,
                       const std::string_view column) {
    return std::string(value_at(document, row, column));
}

bool row_integer(const TsvDocument& document,
                 const std::vector<std::string>& row,
                 const std::string_view column, lua_Integer& result,
                 const std::filesystem::path& path,
                 const std::size_t row_number, std::string& failure) {
    if (parse_integer(value_at(document, row, column), result)) {
        return true;
    }
    failure = path.string() + ": row " + std::to_string(row_number) +
              " column " + std::string(column) + " is not an integer";
    return false;
}

std::vector<std::string> split_flags(std::string_view value) {
    std::vector<std::string> flags;
    while (!value.empty()) {
        const auto comma = value.find(',');
        auto part = value.substr(0U, comma);
        while (!part.empty() &&
               (part.front() == ' ' || part.front() == '\t')) {
            part.remove_prefix(1U);
        }
        while (!part.empty() &&
               (part.back() == ' ' || part.back() == '\t')) {
            part.remove_suffix(1U);
        }
        if (!part.empty()) {
            flags.emplace_back(part);
        }
        if (comma == std::string_view::npos) {
            break;
        }
        value.remove_prefix(comma + 1U);
    }
    return flags;
}

bool push_runtime_event(lua_State* state, const TsvDocument& document,
                        const std::vector<std::string>& row,
                        const int scene_id,
                        const std::filesystem::path& path,
                        const std::size_t row_number,
                        std::set<std::uint32_t>& event_ids,
                        std::size_t& event_count, std::string& failure) {
    lua_Integer event_id = 0;
    if (!row_integer(document, row, "event_id", event_id, path, row_number,
                     failure)) {
        return false;
    }
    if (event_id <= 0) {
        return true;
    }
    if (event_id > std::numeric_limits<std::uint32_t>::max() ||
        !event_ids.insert(static_cast<std::uint32_t>(event_id)).second) {
        failure = path.string() + ": row " + std::to_string(row_number) +
                  " has an invalid or duplicate runtime event ID";
        return false;
    }
    if (++event_count > 512U) {
        failure = "world content exceeds the runtime event limit";
        return false;
    }

    const auto trigger = row_string(document, row, "trigger_type");
    const auto coordinate_space =
        row_string(document, row, "coordinate_space");
    const auto action = row_string(document, row, "action");
    if ((trigger != "scene_entry" && trigger != "actor_within_scope") ||
        (action != "run_lua_handler" &&
         action != "dispatch_native_script")) {
        failure = path.string() + ": row " + std::to_string(row_number) +
                  " has an unsupported runtime trigger or action";
        return false;
    }

    lua_Integer min_x = 0;
    lua_Integer max_x = 0;
    lua_Integer min_y = 0;
    lua_Integer max_y = 0;
    if (coordinate_space == "field_fixed_4") {
        if (!row_integer(document, row, "scope_min_x", min_x, path,
                         row_number, failure) ||
            !row_integer(document, row, "scope_max_x", max_x, path,
                         row_number, failure) ||
            !row_integer(document, row, "scope_min_y", min_y, path,
                         row_number, failure) ||
            !row_integer(document, row, "scope_max_y", max_y, path,
                         row_number, failure)) {
            return false;
        }
    } else if (coordinate_space == "map_pixels") {
        lua_Integer x = 0;
        lua_Integer y = 0;
        lua_Integer width = 0;
        lua_Integer height = 0;
        if (!row_integer(document, row, "x", x, path, row_number,
                         failure) ||
            !row_integer(document, row, "y", y, path, row_number,
                         failure) ||
            !row_integer(document, row, "width", width, path, row_number,
                         failure) ||
            !row_integer(document, row, "height", height, path,
                         row_number, failure)) {
            return false;
        }
        if (x < 0 || y < 0 || width <= 0 || height <= 0 ||
            x > 4095 || y > 4095 || width > 4096 || height > 4096) {
            failure = path.string() + ": row " +
                      std::to_string(row_number) +
                      " has a map-pixel scope outside field coordinates";
            return false;
        }
        min_x = x * 16;
        max_x = (x + width) * 16 - 1;
        min_y = y * 16;
        max_y = (y + height) * 16 - 1;
    } else {
        failure = path.string() + ": row " + std::to_string(row_number) +
                  " has an unsupported coordinate space";
        return false;
    }
    if (min_x < 0 || min_y < 0 || max_x < min_x || max_y < min_y ||
        max_x > 65535 || max_y > 65535) {
        failure = path.string() + ": row " + std::to_string(row_number) +
                  " has an invalid source-coordinate scope";
        return false;
    }
    const auto required_flags =
        split_flags(value_at(document, row, "required_flags"));
    for (const auto& flag : required_flags) {
        if (flag != "new_game_opening_completed") {
            failure = path.string() + ": row " +
                      std::to_string(row_number) +
                      " has an unsupported runtime state flag";
            return false;
        }
    }
    const auto handler = row_string(document, row, "script_function");
    if (action == "run_lua_handler" && handler.empty()) {
        failure = path.string() + ": row " + std::to_string(row_number) +
                  " has no Lua handler";
        return false;
    }

    lua_createtable(state, 0, 8);
    const auto event_table = lua_absindex(state, -1);
    lua_pushinteger(state, event_id);
    lua_setfield(state, event_table, "id");
    lua_pushinteger(state, scene_id);
    lua_setfield(state, event_table, "scene");
    lua_pushlstring(state, trigger.data(), trigger.size());
    lua_setfield(state, event_table, "trigger");
    lua_createtable(state, 0, 4);
    lua_pushinteger(state, min_x);
    lua_setfield(state, -2, "min_x");
    lua_pushinteger(state, max_x);
    lua_setfield(state, -2, "max_x");
    lua_pushinteger(state, min_y);
    lua_setfield(state, -2, "min_y");
    lua_pushinteger(state, max_y);
    lua_setfield(state, -2, "max_y");
    lua_setfield(state, event_table, "scope");
    lua_createtable(state, static_cast<int>(required_flags.size()), 0);
    for (std::size_t index = 0; index < required_flags.size(); ++index) {
        const auto& flag = required_flags[index];
        lua_pushlstring(state, flag.data(), flag.size());
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    lua_setfield(state, event_table, "requires");
    const auto repeat = row_string(document, row, "repeat_policy");
    lua_pushlstring(state, repeat.data(), repeat.size());
    lua_setfield(state, event_table, "repeat");
    lua_pushlstring(state, action.data(), action.size());
    lua_setfield(state, event_table, "action");
    if (action == "run_lua_handler") {
        lua_pushlstring(state, handler.data(), handler.size());
        lua_setfield(state, event_table, "handler");
    }
    return true;
}

bool push_world_areas(lua_State* state,
                      const std::filesystem::path& world_root,
                      const int root_table, std::string& failure) {
    const auto area_root = world_root / "areas";
    std::error_code error;
    if (!std::filesystem::is_directory(area_root, error)) {
        if (!error) {
            failure = "world area directory is missing: " +
                      area_root.string();
        } else {
            failure = "could not inspect world area directory: " +
                      error.message();
        }
        return false;
    }
    std::map<int, std::map<std::string, std::filesystem::path>> scene_files;
    for (std::filesystem::directory_iterator iterator(area_root, error), end;
         !error && iterator != end; iterator.increment(error)) {
        if (!iterator->is_regular_file(error)) {
            continue;
        }
        int scene_id = 0;
        std::string suffix;
        if (scene_file(iterator->path(), scene_id, suffix)) {
            scene_files[scene_id][suffix] = iterator->path();
        }
    }
    if (error) {
        failure = "could not enumerate world area data: " + error.message();
        return false;
    }

    lua_createtable(state, 0, 6);
    const auto areas_table = lua_absindex(state, -1);
    lua_createtable(state, 0, 6);
    const auto triggers_table = lua_absindex(state, -1);
    lua_createtable(state, 0, 6);
    const auto spawns_table = lua_absindex(state, -1);
    lua_createtable(state, 0, 6);
    const auto encounters_table = lua_absindex(state, -1);
    lua_createtable(state, 0, 8);
    const auto events_table = lua_absindex(state, -1);
    std::set<std::uint32_t> event_ids;
    std::size_t event_count = 0U;

    for (const auto& [scene_id, files] : scene_files) {
        TsvDocument areas;
        const auto area_path = files.find("");
        const auto resolved_area_path =
            area_path == files.end()
                ? area_root / ("scene-" +
                               (scene_id < 10 ? std::string("0") : "") +
                               std::to_string(scene_id) + ".tsv")
                : area_path->second;
        if (!read_tsv(resolved_area_path, true, areas, failure) ||
            !push_rows(state, areas,
                       {"origin_x", "origin_y"}, resolved_area_path,
                       failure)) {
            lua_pop(state, 5);
            return false;
        }
        lua_rawseti(state, areas_table, scene_id);

        struct SceneDataFile {
            const char* suffix;
            const char* module_key;
            std::vector<std::string_view> numeric_columns;
            std::vector<std::pair<std::string_view, std::string_view>>
                aliases;
        };
        const std::vector<SceneDataFile> data_files{
            {"triggers", "map_triggers",
             {"event_id", "scope_min_x", "scope_max_x", "scope_min_y",
              "scope_max_y", "x", "y", "width", "height"},
             {{"trigger_id", "id"}}},
            {"spawns", "actor_spawns", {"x", "y"},
             {{"spawn_id", "id"}}},
            {"encounters", "encounter_regions",
             {"x", "y", "width", "height"},
             {{"region_id", "id"}}},
        };
        for (const auto& data_file : data_files) {
            TsvDocument document;
            const auto data_path = files.find(data_file.suffix);
            const auto resolved_path =
                data_path == files.end()
                    ? area_root / ("scene-" +
                                   (scene_id < 10 ? std::string("0") : "") +
                                   std::to_string(scene_id) + "-" +
                                   data_file.suffix + ".tsv")
                    : data_path->second;
            if (!read_tsv(resolved_path, true, document, failure) ||
                !push_rows(state, document, data_file.numeric_columns,
                           resolved_path, failure, data_file.aliases)) {
                lua_pop(state, 5);
                return false;
            }
            lua_rawseti(
                state,
                std::string_view(data_file.module_key) == "map_triggers"
                    ? triggers_table
                    : (std::string_view(data_file.module_key) == "actor_spawns"
                           ? spawns_table
                           : encounters_table),
                scene_id);

            if (std::string_view(data_file.suffix) != "triggers") {
                continue;
            }
            for (std::size_t row_index = 0; row_index < document.rows.size();
                 ++row_index) {
                const auto event_table_base = lua_gettop(state);
                const bool has_event = [&] {
                    lua_Integer value = 0;
                    return row_integer(document, document.rows[row_index],
                                       "event_id", value, resolved_path,
                                       row_index + 2U, failure) &&
                           value > 0;
                }();
                if (!has_event) {
                    if (!failure.empty()) {
                        lua_settop(state, event_table_base);
                        lua_pop(state, 5);
                        return false;
                    }
                    continue;
                }
                if (!push_runtime_event(
                        state, document, document.rows[row_index], scene_id,
                        resolved_path, row_index + 2U, event_ids, event_count,
                        failure)) {
                    lua_settop(state, event_table_base);
                    lua_pop(state, 5);
                    return false;
                }
                lua_rawseti(state, events_table,
                            static_cast<lua_Integer>(event_count));
            }
        }
    }

    lua_pushvalue(state, areas_table);
    lua_setfield(state, root_table, "areas");
    lua_pushvalue(state, triggers_table);
    lua_setfield(state, root_table, "map_triggers");
    lua_pushvalue(state, spawns_table);
    lua_setfield(state, root_table, "actor_spawns");
    lua_pushvalue(state, encounters_table);
    lua_setfield(state, root_table, "encounter_regions");
    lua_pushvalue(state, events_table);
    lua_setfield(state, root_table, "map_events");
    lua_pop(state, 5);
    return true;
}

void push_empty_data(lua_State* state, const int modules_reference) {
    lua_createtable(state, 0, 6);
    const auto root_table = lua_absindex(state, -1);
    lua_createtable(state, 0, 0);
    lua_setfield(state, root_table, "entities");
    lua_createtable(state, 0, 0);
    lua_setfield(state, root_table, "areas");
    lua_createtable(state, 0, 0);
    lua_setfield(state, root_table, "map_triggers");
    lua_createtable(state, 0, 0);
    lua_setfield(state, root_table, "actor_spawns");
    lua_createtable(state, 0, 0);
    lua_setfield(state, root_table, "encounter_regions");
    lua_createtable(state, 0, 0);
    lua_setfield(state, root_table, "map_events");
    lua_rawgeti(state, LUA_REGISTRYINDEX, modules_reference);
    lua_pushvalue(state, root_table);
    lua_setfield(state, -2, "world_content_data");
    lua_pop(state, 2);
}

} // namespace

bool load_world_content_tsv(lua_State* state, const int modules_reference,
                            const std::filesystem::path& world_directory,
                            std::string& failure) {
    if (state == nullptr || modules_reference == LUA_NOREF) {
        failure = "Lua runtime is unavailable for world-content TSV files";
        return false;
    }
    std::error_code error;
    if (!std::filesystem::exists(world_directory, error)) {
        if (error) {
            failure = "could not inspect world-content directory: " +
                      error.message();
            return false;
        }
        push_empty_data(state, modules_reference);
        return true;
    }
    if (!std::filesystem::is_directory(world_directory, error) || error) {
        failure = "world-content path is not a directory: " +
                  world_directory.string();
        return false;
    }

    const auto stack_base = lua_gettop(state);
    lua_createtable(state, 0, 6);
    const auto root_table = lua_absindex(state, -1);
    if (!push_entities(state, world_directory, failure)) {
        lua_settop(state, stack_base);
        return false;
    }
    lua_setfield(state, root_table, "entities");
    if (!push_world_areas(state, world_directory, root_table, failure)) {
        lua_settop(state, stack_base);
        return false;
    }
    lua_rawgeti(state, LUA_REGISTRYINDEX, modules_reference);
    lua_pushvalue(state, root_table);
    lua_setfield(state, -2, "world_content_data");
    lua_settop(state, stack_base);
    return true;
}

} // namespace shadowrun::ghidra_conversion::scripting::detail
