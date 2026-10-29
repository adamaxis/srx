#include "world_content_tsv_loader.hpp"
#include "enums.hpp"
#include "shadowrun_ghidra/scripting/combat_inventory.hpp"
#include "shadowrun_ghidra/scripting/weapon_action_policy.hpp"

#include <algorithm>
#include <array>
#include <charconv>
#include <cstdint>
#include <fstream>
#include <initializer_list>
#include <limits>
#include <map>
#include <set>
#include <string_view>
#include <system_error>
#include <tuple>
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

bool read_source_byte(const TsvDocument& document,
                      const std::vector<std::string>& row,
                      const std::string_view column,
                      const std::filesystem::path& source_path,
                      const std::size_t row_number, std::uint8_t& result,
                      std::string& failure) {
    lua_Integer parsed = 0;
    if (!parse_integer(value_at(document, row, column), parsed) ||
        parsed < 0 || parsed > 0xFF) {
        failure = source_path.string() + ": row " +
                  std::to_string(row_number) + " column " +
                  std::string(column) + " is not a source byte";
        return false;
    }
    result = static_cast<std::uint8_t>(parsed);
    return true;
}

bool push_weapon_action_policies(
    lua_State* state, const TsvDocument& document,
    const std::vector<std::string>& row,
    const std::filesystem::path& source_path,
    const std::size_t row_number, std::string& failure) {
    if (document.column_indexes.find("combat_item_phase_0") ==
        document.column_indexes.end()) {
        return true;
    }
    std::array<std::uint8_t, 12> source_modifiers{};
    for (std::size_t index = 0U; index < 8U; ++index) {
        const auto column = "combat_item_phase_" + std::to_string(index);
        if (!read_source_byte(document, row, column, source_path, row_number,
                              source_modifiers[index], failure)) {
            return false;
        }
    }
    constexpr std::array<std::string_view, 4> extended_columns{{
        "combat_action_phase_76", "combat_action_phase_77",
        "combat_action_phase_78", "combat_action_phase_79"}};
    for (std::size_t index = 0U; index < extended_columns.size(); ++index) {
        if (!read_source_byte(document, row, extended_columns[index],
                              source_path, row_number,
                              source_modifiers[index + 8U], failure)) {
            return false;
        }
    }

    lua_createtable(state, 0,
                    static_cast<int>(kScriptWeaponActionIds.size()));
    const auto policies_index = lua_absindex(state, -1);
    for (std::size_t index = 0U; index < kScriptWeaponActionIds.size();
         ++index) {
        const auto action_id = kScriptWeaponActionIds[index];
        const auto policy = script_weapon_action_policy(
            action_id, source_modifiers[index]);
        lua_createtable(state, 0, 3);
        const auto skill_group =
            script_weapon_skill_group_name(policy.skill_group);
        lua_pushlstring(state, skill_group.data(), skill_group.size());
        lua_setfield(state, -2, "skill_group");
        lua_pushinteger(state, policy.difficulty_adjustment);
        lua_setfield(state, -2, "difficulty_adjustment");
        lua_pushboolean(state, policy.benefits_from_targeting_assist);
        lua_setfield(state, -2, "benefits_from_targeting_assist");
        const auto action_name = script_weapon_action_name(action_id);
        lua_setfield(state, policies_index, action_name.data());
    }
    lua_setfield(state, -2, "weapon_action_policies");
    return true;
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

bool push_semantic_inventory(
    lua_State* state, const TsvDocument& document,
    const std::vector<std::string>& row,
    const std::filesystem::path& source_path,
    const std::size_t row_number, std::string& failure) {
    if (document.column_indexes.find("combat_item_code_0") ==
        document.column_indexes.end()) {
        return true;
    }
    lua_createtable(state, 8, 0);
    const auto inventory_index = lua_absindex(state, -1);
    lua_Integer inventory_entry = 1;
    for (std::size_t slot = 0U; slot < 8U; ++slot) {
        const auto code_column = "combat_item_code_" + std::to_string(slot);
        const auto value_column = "combat_item_phase_" + std::to_string(slot);
        std::uint8_t item_code = 0U;
        std::uint8_t item_value = 0U;
        if (!read_source_byte(document, row, code_column, source_path,
                              row_number, item_code, failure) ||
            !read_source_byte(document, row, value_column, source_path,
                              row_number, item_value, failure)) {
            lua_pop(state, 1);
            return false;
        }
        if (item_code == 0U) {
            continue;
        }
        const auto item = script_combat_inventory_item(
            static_cast<std::uint8_t>(slot + 1U), item_code, item_value);
        if (!item.has_value()) {
            failure = source_path.string() + ": row " +
                      std::to_string(row_number) + " has unknown inventory "
                      "item code " + std::to_string(item_code);
            lua_pop(state, 1);
            return false;
        }
        if (item->value_property.empty() && item_value != 0U) {
            failure = source_path.string() + ": row " +
                      std::to_string(row_number) + " has an unmapped nonzero "
                      "inventory value for " + std::string(item->item_id);
            lua_pop(state, 1);
            return false;
        }
        lua_createtable(state, 0, item->value_property.empty() ? 2 : 3);
        const auto item_index = lua_absindex(state, -1);
        lua_pushlstring(state, item->item_id.data(), item->item_id.size());
        lua_setfield(state, item_index, "id");
        lua_pushinteger(state, item->slot);
        lua_setfield(state, item_index, "slot");
        if (!item->value_property.empty()) {
            lua_pushinteger(state, item->value);
            lua_setfield(state, item_index,
                         std::string(item->value_property).c_str());
        }
        lua_rawseti(state, inventory_index, inventory_entry++);
    }
    lua_setfield(state, -2, "inventory");
    return true;
}

bool push_record(lua_State* state, const TsvDocument& document,
                 const std::vector<std::string>& row,
                 const std::vector<std::string_view>& numeric_columns,
                 const std::vector<
                     std::pair<std::string_view, std::string_view>>& aliases,
                 const std::filesystem::path& source_path,
                 const std::size_t row_number, std::string& failure,
                 const std::vector<std::string_view>& optional_numeric_columns = {},
                 const std::vector<std::string_view>& boolean_columns = {}) {
    lua_createtable(state, 0, static_cast<int>(document.columns.size()));
    const auto table_index = lua_absindex(state, -1);
    for (const auto& column : document.columns) {
        // These source bytes are import evidence. Lua receives named actor
        // traits and interaction rules built below, never the packed byte.
        if (column == "source_spawn_state_flags" ||
            column == "source_interaction_flags") {
            continue;
        }
        if (column.rfind("combat_item_code_", 0U) == 0U ||
            column.rfind("combat_item_phase_", 0U) == 0U) {
            continue;
        }
        if (column.rfind("combat_action_phase_", 0U) == 0U) {
            if (column == "combat_action_phase_7f") {
                std::uint8_t magic_cap = 0U;
                if (!read_source_byte(document, row, column, source_path,
                                      row_number, magic_cap, failure)) {
                    lua_pop(state, 1);
                    return false;
                }
                lua_pushinteger(state, magic_cap);
                lua_setfield(state, table_index, "combat_magic_cap");
            }
            continue;
        }
        const auto value = value_at(document, row, column);
        if (numeric_column(column, boolean_columns)) {
            if (value == "true") {
                lua_pushboolean(state, 1);
            } else if (value == "false") {
                lua_pushboolean(state, 0);
            } else {
                failure = source_path.string() + ": row " +
                          std::to_string(row_number) + " column " + column +
                          " is not a boolean";
                lua_pop(state, 1);
                return false;
            }
        } else if (numeric_column(column, optional_numeric_columns)) {
            if (value.empty() || value == "-") {
                lua_pushnil(state);
            } else {
                lua_Integer number = 0;
                if (!parse_integer(value, number)) {
                    failure = source_path.string() + ": row " +
                              std::to_string(row_number) + " column " + column +
                              " is not an integer";
                    lua_pop(state, 1);
                    return false;
                }
                lua_pushinteger(state, number);
            }
        } else if (numeric_column(column, numeric_columns)) {
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

    if (!push_weapon_action_policies(state, document, row, source_path,
                                     row_number, failure)) {
        lua_pop(state, 1);
        return false;
    }
    if (!push_semantic_inventory(state, document, row, source_path,
                                 row_number, failure)) {
        lua_pop(state, 1);
        return false;
    }

    const auto push_actor_group = [state](
        const char* group_name,
        const std::initializer_list<std::pair<const char*, std::uint8_t>> fields,
        const std::uint8_t value) {
        lua_createtable(state, 0, static_cast<int>(fields.size()));
        const auto group_index = lua_absindex(state, -1);
        for (const auto& [name, mask] : fields) {
            lua_pushboolean(state, (value & mask) != 0U);
            lua_setfield(state, group_index, name);
        }
        lua_setfield(state, -2, group_name);
    };
    const auto push_semantic_actor_facts = [&]() -> bool {
        const bool has_roles = document.column_indexes.find(
            "source_spawn_state_flags") != document.column_indexes.end();
        const bool has_interaction = document.column_indexes.find(
            "source_interaction_flags") != document.column_indexes.end();
        if (!has_roles && !has_interaction) {
            return true;
        }
        lua_createtable(state, 0, 2);
        const auto facts_index = lua_absindex(state, -1);
        const auto read_byte = [&](const std::string_view column,
                                   std::uint8_t& output) {
            lua_Integer parsed = 0;
            const auto text = value_at(document, row, column);
            if (!parse_integer(text, parsed) || parsed < 0 || parsed > 0xFF) {
                failure = source_path.string() + ": row " +
                          std::to_string(row_number) + " column " +
                          std::string(column) + " is not an actor fact byte";
                return false;
            }
            output = static_cast<std::uint8_t>(parsed);
            return true;
        };
        std::uint8_t roles = 0U;
        std::uint8_t interaction = 0U;
        if (has_roles && !read_byte("source_spawn_state_flags", roles)) {
            lua_pop(state, 1);
            return false;
        }
        if (has_interaction &&
            !read_byte("source_interaction_flags", interaction)) {
            lua_pop(state, 1);
            return false;
        }
        if (has_roles) {
            push_actor_group("roles",
                {{"spotted", 0x01U}, {"reinforcement", 0x02U},
                 {"despawn_offscreen", 0x04U}, {"quest", 0x08U},
                 {"runner", 0x10U}, {"pedestrian", 0x20U},
                 {"hostile", 0x40U}}, roles);
        }
        if (has_interaction) {
            push_actor_group("interaction",
                {{"projectile", 0x01U},
                 {"proximity_interaction_pending", 0x02U},
                 {"use_collision_heading", 0x04U},
                 {"collision_recovery_active", 0x08U},
                 {"action_hook_pending", 0x10U},
                 {"defense_difficulty_reduction", 0x20U},
                 {"breathing_out", 0x40U}, {"unseen", 0x80U},
                 {"no_coverage_difficulty_bonus", 0x80U}}, interaction);
        }
        lua_setfield(state, table_index, "actor_facts");
        return true;
    };
    if (!push_semantic_actor_facts()) {
        lua_pop(state, 1);
        return false;
    }
    return true;
}

bool push_rows(lua_State* state, const TsvDocument& document,
               const std::vector<std::string_view>& numeric_columns,
               const std::filesystem::path& source_path,
               std::string& failure,
               const std::vector<
                   std::pair<std::string_view, std::string_view>>& aliases = {},
               const std::vector<std::string_view>& optional_numeric_columns = {},
               const std::vector<std::string_view>& boolean_columns = {}) {
    lua_createtable(state, static_cast<int>(document.rows.size()), 0);
    const auto array_index = lua_absindex(state, -1);
    for (std::size_t index = 0; index < document.rows.size(); ++index) {
        if (!push_record(state, document, document.rows[index],
                         numeric_columns, aliases, source_path, index + 2U,
                         failure, optional_numeric_columns, boolean_columns)) {
            lua_pop(state, 1);
            return false;
        }
        lua_rawseti(state, array_index, static_cast<lua_Integer>(index + 1U));
    }
    return true;
}

bool row_integer(const TsvDocument& document,
                 const std::vector<std::string>& row,
                 std::string_view column, lua_Integer& result,
                 const std::filesystem::path& path,
                 std::size_t row_number, std::string& failure);

bool push_scene_indexed_rows(
    lua_State* state, const TsvDocument& document,
    const std::vector<std::string_view>& numeric_columns,
    const std::filesystem::path& source_path, std::string& failure,
    const bool arrays_by_scene) {
    std::map<lua_Integer, std::vector<std::size_t>> rows_by_scene;
    for (std::size_t row_index = 0U; row_index < document.rows.size();
         ++row_index) {
        lua_Integer scene_id = 0;
        if (!row_integer(document, document.rows[row_index], "scene_id",
                         scene_id, source_path, row_index + 2U, failure) ||
            scene_id < 0 || scene_id > srx::v(srx::Scene::LAST_ID)) {
            if (failure.empty()) {
                failure = source_path.string() + ": row " +
                    std::to_string(row_index + 2U) +
                    " has a scene_id outside the ROM scene range";
            }
            return false;
        }
        rows_by_scene[scene_id].push_back(row_index);
    }

    lua_createtable(state, 0, static_cast<int>(rows_by_scene.size()));
    const auto scenes_index = lua_absindex(state, -1);
    for (const auto& [scene_id, row_indexes] : rows_by_scene) {
        if (!arrays_by_scene && row_indexes.size() != 1U) {
            failure = source_path.string() +
                ": expected one profile row per scene";
            lua_pop(state, 1);
            return false;
        }
        if (arrays_by_scene) {
            lua_createtable(state, static_cast<int>(row_indexes.size()), 0);
            const auto rows_index = lua_absindex(state, -1);
            for (std::size_t output_index = 0U;
                 output_index < row_indexes.size(); ++output_index) {
                const auto row_index = row_indexes[output_index];
                if (!push_record(state, document, document.rows[row_index],
                                 numeric_columns, {}, source_path,
                                 row_index + 2U, failure)) {
                    lua_pop(state, 2);
                    return false;
                }
                lua_rawseti(state, rows_index,
                            static_cast<lua_Integer>(output_index + 1U));
            }
        } else {
            const auto row_index = row_indexes.front();
            if (!push_record(state, document, document.rows[row_index],
                             numeric_columns, {}, source_path,
                             row_index + 2U, failure)) {
                lua_pop(state, 1);
                return false;
            }
        }
        lua_pushinteger(state, scene_id);
        lua_insert(state, -2);
        lua_settable(state, scenes_index);
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

using ActivityListKey = std::pair<lua_Integer, lua_Integer>;
using ActivityListIds = std::map<ActivityListKey, std::string>;
using ActorProfileKey = std::tuple<lua_Integer, lua_Integer, lua_Integer>;
using ActorProfileIds = std::map<ActorProfileKey, std::string>;
using SourceSequenceIds = std::map<lua_Integer, std::string>;

bool load_source_sequence_ids(
    const std::filesystem::path& world_directory,
    SourceSequenceIds& sequence_ids, std::string& failure) {
    const auto path = world_directory / "field_actor_sequences.tsv";
    TsvDocument document;
    if (!read_tsv(path, true, document, failure)) {
        return false;
    }
    for (std::size_t index = 0U; index < document.rows.size(); ++index) {
        const auto& row = document.rows[index];
        lua_Integer source_list_offset = 0;
        if (!row_integer(document, row, "source_list_offset",
                         source_list_offset, path, index + 2U, failure) ||
            source_list_offset < 0 || source_list_offset > 0x1FFFFFU) {
            if (failure.empty()) {
                failure = path.string() + ": row " +
                    std::to_string(index + 2U) +
                    " has an invalid source sequence key";
            }
            return false;
        }
        const auto sequence_id = row_string(document, row, "sequence_id");
        const bool valid_id = !sequence_id.empty() &&
            std::all_of(sequence_id.begin(), sequence_id.end(),
                        [](const char character) {
                            return (character >= 'a' && character <= 'z') ||
                                   (character >= 'A' && character <= 'Z') ||
                                   (character >= '0' && character <= '9') ||
                                   character == '_' || character == '-' ||
                                   character == '.' || character == ':';
                        });
        if (!valid_id) {
            failure = path.string() + ": row " +
                std::to_string(index + 2U) +
                " has an invalid semantic sequence id";
            return false;
        }
        if (!sequence_ids.emplace(source_list_offset, sequence_id).second) {
            failure = path.string() + ": source sequence keys are duplicated";
            return false;
        }
    }
    return true;
}

bool collect_activity_list_ids(
    const TsvDocument& document, const std::filesystem::path& path,
    ActivityListIds& activity_ids, std::string& failure) {
    for (std::size_t index = 0U; index < document.rows.size(); ++index) {
        const auto& row = document.rows[index];
        lua_Integer scene_id = 0;
        lua_Integer list_offset = 0;
        if (!row_integer(document, row, "scene", scene_id, path, index + 2U,
                         failure) ||
            !row_integer(document, row, "list_offset", list_offset, path,
                         index + 2U, failure) ||
            scene_id < 0 || scene_id > srx::v(srx::Scene::LAST_ID) ||
            list_offset < 0 || list_offset > 0x1FFFFF) {
            if (failure.empty()) {
                failure = path.string() + ": row " +
                    std::to_string(index + 2U) +
                    " has an activity-list key outside source bounds";
            }
            return false;
        }
        const auto activity_id = row_string(document, row, "list_id");
        if (activity_id.empty() || activity_id.size() > 128U) {
            failure = path.string() + ": row " +
                std::to_string(index + 2U) +
                " has no stable activity id";
            return false;
        }
        const ActivityListKey key{scene_id, list_offset};
        const auto [found, inserted] =
            activity_ids.emplace(key, activity_id);
        if (!inserted && found->second != activity_id) {
            failure = path.string() + ": scene/list pair maps to multiple "
                      "activity ids";
            return false;
        }
    }
    return true;
}

std::string_view semantic_actor_combat_column(
    const std::string_view source_column) {
    static constexpr std::array<std::pair<std::string_view,
                                          std::string_view>, 11U> aliases{{
        {"combat_attribute_77", "combat_body"},
        {"combat_attribute_78", "combat_quickness"},
        {"combat_attribute_79", "combat_strength"},
        {"combat_attribute_7b", "combat_intelligence"},
        {"combat_attribute_7c", "combat_willpower"},
        {"combat_attribute_85", "combat_melee_combat"},
        {"combat_item18_base", "combat_sorcery"},
        {"combat_action_skill_81", "combat_firearms"},
        {"combat_action_skill_82", "combat_pistols"},
        {"combat_action_skill_83", "combat_smgs"},
        {"combat_action_skill_84", "combat_shotguns"},
    }};
    for (const auto& [source, semantic] : aliases) {
        if (source_column == source) {
            return semantic;
        }
    }
    return source_column;
}

bool make_semantic_activity_actor_rows(
    const TsvDocument& source, const std::filesystem::path& path,
    TsvDocument& semantic, std::string& failure) {
    const auto tag_column = source.column_indexes.find("tagged_template");
    if (tag_column == source.column_indexes.end()) {
        failure = path.string() + ": actor catalog has no template tag";
        return false;
    }

    semantic = {};
    semantic.columns.reserve(source.columns.size());
    for (const auto& source_column : source.columns) {
        if (source_column == "source_record_offset" ||
            source_column == "list_offset" ||
            source_column == "position_offset" ||
            source_column == "template_offset") {
            continue;
        }
        const auto runtime_column = source_column == "tagged_template"
            ? std::string("activity_probe_enabled")
            : std::string(semantic_actor_combat_column(source_column));
        const auto index = semantic.columns.size();
        if (!semantic.column_indexes.emplace(runtime_column, index).second) {
            failure = path.string() +
                ": actor catalog semantic columns contain a duplicate name";
            return false;
        }
        semantic.columns.push_back(runtime_column);
    }

    semantic.rows.reserve(source.rows.size());
    for (std::size_t row_index = 0U; row_index < source.rows.size();
         ++row_index) {
        const auto& source_row = source.rows[row_index];
        lua_Integer tagged_template = 0;
        if (tag_column->second >= source_row.size() ||
            !parse_integer(source_row[tag_column->second], tagged_template) ||
            tagged_template < 0 ||
            static_cast<std::uint64_t>(tagged_template) >
                std::numeric_limits<std::uint32_t>::max()) {
            failure = path.string() + ": row " +
                std::to_string(row_index + 2U) +
                " has an invalid template tag";
            return false;
        }

        std::vector<std::string> runtime_row;
        runtime_row.reserve(semantic.columns.size());
        for (std::size_t column = 0U; column < source.columns.size();
             ++column) {
            if (source.columns[column] == "source_record_offset" ||
                source.columns[column] == "list_offset" ||
                source.columns[column] == "position_offset" ||
                source.columns[column] == "template_offset") {
                continue;
            }
            if (column == tag_column->second) {
                // Preserve the current host interpretation without exposing
                // the tagged source pointer to Lua.
                runtime_row.emplace_back(
                    (static_cast<std::uint32_t>(tagged_template) &
                     0x80000000U) != 0U ? "true" : "false");
            } else {
                runtime_row.push_back(source_row[column]);
            }
        }
        semantic.rows.push_back(std::move(runtime_row));
    }
    return true;
}

bool make_semantic_dynamic_actor_rows(
    const TsvDocument& source, const std::filesystem::path& path,
    const SourceSequenceIds& source_sequence_ids, TsvDocument& semantic,
    std::string& failure) {
    const auto scene_column = source.column_indexes.find("scene");
    const auto context_column = source.column_indexes.find("context_group");
    const auto sum_column = source.column_indexes.find("random_sum");
    const auto list_column = source.column_indexes.find("sequence_list_offset");
    const auto tag_column = source.column_indexes.find("tagged_template");
    const auto run_target_area_column =
        source.column_indexes.find("run_target_area");
    const auto run_target_event_column =
        source.column_indexes.find("run_target_event");
    if (scene_column == source.column_indexes.end() ||
        context_column == source.column_indexes.end() ||
        sum_column == source.column_indexes.end() ||
        list_column == source.column_indexes.end() ||
        tag_column == source.column_indexes.end() ||
        run_target_area_column == source.column_indexes.end() ||
        run_target_event_column == source.column_indexes.end()) {
        failure = path.string() +
            ": dynamic actor catalog is missing source selection metadata";
        return false;
    }

    semantic = {};
    semantic.columns.reserve(source.columns.size());
    for (const auto& source_column : source.columns) {
        if (source_column == "selection_cell_offset" ||
            source_column == "sequence_list_offset" ||
            source_column == "template_offset") {
            continue;
        }
        const auto runtime_column = source_column == "tagged_template"
            ? std::string("activity_probe_enabled")
            : std::string(semantic_actor_combat_column(source_column));
        const auto index = semantic.columns.size();
        if (!semantic.column_indexes.emplace(runtime_column, index).second) {
            failure = path.string() +
                ": dynamic actor semantic columns contain a duplicate name";
            return false;
        }
        semantic.columns.push_back(runtime_column);
        if (source_column == "random_sum") {
            const auto sequence_index = semantic.columns.size();
            semantic.column_indexes.emplace("sequence_id", sequence_index);
            semantic.columns.emplace_back("sequence_id");
        }
    }

    semantic.rows.reserve(source.rows.size());
    for (std::size_t row_index = 0U; row_index < source.rows.size();
         ++row_index) {
        const auto& source_row = source.rows[row_index];
        const auto parse_column = [&](const std::size_t column,
                                      const std::string_view name,
                                      lua_Integer& value) {
            if (column >= source_row.size() ||
                parse_integer(source_row[column], value)) {
                return column < source_row.size();
            }
            failure = path.string() + ": row " +
                std::to_string(row_index + 2U) + " column " +
                std::string(name) + " is not an integer";
            return false;
        };
        lua_Integer scene_id = 0;
        lua_Integer context_group = 0;
        lua_Integer random_sum = 0;
        lua_Integer sequence_list_offset = 0;
        lua_Integer tagged_template = 0;
        lua_Integer run_target_area = 0;
        lua_Integer run_target_event = 0;
        if (!parse_column(scene_column->second, "scene", scene_id) ||
            !parse_column(context_column->second, "context_group",
                          context_group) ||
            !parse_column(sum_column->second, "random_sum", random_sum) ||
            !parse_column(list_column->second, "sequence_list_offset",
                          sequence_list_offset) ||
            !parse_column(tag_column->second, "tagged_template",
                          tagged_template) ||
            !parse_column(run_target_area_column->second, "run_target_area",
                          run_target_area) ||
            !parse_column(run_target_event_column->second, "run_target_event",
                          run_target_event) ||
            scene_id < 0 || scene_id > srx::v(srx::Scene::LAST_ID) ||
            context_group < 0 || context_group > 0xFF ||
            random_sum < 0 || random_sum > 0xFF ||
            sequence_list_offset < 0 || sequence_list_offset > 0x1FFFFFU ||
            run_target_area < 0 || run_target_area > 0xFF ||
            run_target_event < 0 || run_target_event > 0xFF ||
            tagged_template < 0 ||
            static_cast<std::uint64_t>(tagged_template) >
                std::numeric_limits<std::uint32_t>::max()) {
            if (failure.empty()) {
                failure = path.string() + ": row " +
                    std::to_string(row_index + 2U) +
                    " has invalid dynamic actor selector facts";
            }
            return false;
        }

        std::string sequence_id;
        if (random_sum == 0xFF) {
            const auto found = source_sequence_ids.find(sequence_list_offset);
            if (found == source_sequence_ids.end()) {
                failure = path.string() + ": row " +
                    std::to_string(row_index + 2U) +
                    " has no semantic id for its named actor sequence";
                return false;
            }
            sequence_id = found->second;
        } else if (run_target_area != 0) {
            sequence_id = "field_encounter.scene.15.context." +
                std::to_string(run_target_area) + ".weight." +
                std::to_string(run_target_event);
        } else {
            sequence_id = "field_encounter.scene." +
                std::to_string(scene_id) + ".context." +
                std::to_string(context_group) + ".weight." +
                std::to_string(random_sum);
        }

        std::vector<std::string> runtime_row;
        runtime_row.reserve(semantic.columns.size());
        for (std::size_t column = 0U; column < source.columns.size();
             ++column) {
            const auto& source_column = source.columns[column];
            if (source_column == "selection_cell_offset" ||
                source_column == "sequence_list_offset" ||
                source_column == "template_offset") {
                continue;
            }
            if (column == tag_column->second) {
                // Keep the source interpretation as a named fact, never as
                // the high-bit tagged template pointer.
                runtime_row.emplace_back(
                    (static_cast<std::uint32_t>(tagged_template) &
                     0x80000000U) != 0U ? "true" : "false");
            } else {
                runtime_row.push_back(source_row[column]);
            }
            if (column == sum_column->second) {
                runtime_row.push_back(sequence_id);
            }
        }
        semantic.rows.push_back(std::move(runtime_row));
    }
    return true;
}

bool collect_actor_profile_ids(
    const TsvDocument& source, const std::filesystem::path& path,
    ActorProfileIds& profile_ids, std::string& failure) {
    for (std::size_t index = 0U; index < source.rows.size(); ++index) {
        const auto& row = source.rows[index];
        lua_Integer template_offset = 0;
        lua_Integer entity_kind = 0;
        lua_Integer palette_line = 0;
        if (!row_integer(source, row, "template_offset", template_offset,
                         path, index + 2U, failure) ||
            !row_integer(source, row, "entity_kind", entity_kind, path,
                         index + 2U, failure) ||
            !row_integer(source, row, "palette_line", palette_line, path,
                         index + 2U, failure) ||
            template_offset < 0 || template_offset > 0x1FFFFFU ||
            entity_kind < 0 || entity_kind > 15 ||
            palette_line < 0 || palette_line > 3) {
            if (failure.empty()) {
                failure = path.string() + ": row " +
                    std::to_string(index + 2U) +
                    " has an invalid actor profile key";
            }
            return false;
        }
        const auto profile_id = row_string(source, row, "profile_id");
        if (profile_id.empty() || profile_id == "-") {
            failure = path.string() + ": row " +
                std::to_string(index + 2U) +
                " has no semantic actor profile id";
            return false;
        }
        const ActorProfileKey key{template_offset, entity_kind, palette_line};
        const auto [found, inserted] = profile_ids.emplace(key, profile_id);
        if (!inserted && found->second != profile_id) {
            failure = path.string() + ": actor profile key maps to multiple "
                      "profile ids";
            return false;
        }
    }
    return true;
}

bool make_semantic_conditional_actor_rows(
    const TsvDocument& source, const std::filesystem::path& path,
    const ActorProfileIds& profile_ids, TsvDocument& semantic,
    std::string& failure) {
    const auto template_column = source.column_indexes.find("template_offset");
    const auto entity_column = source.column_indexes.find("entity_kind");
    const auto palette_column = source.column_indexes.find("palette_line");
    if (template_column == source.column_indexes.end() ||
        entity_column == source.column_indexes.end() ||
        palette_column == source.column_indexes.end()) {
        failure = path.string() + ": candidate table has no actor profile key";
        return false;
    }

    semantic = {};
    semantic.columns.reserve(source.columns.size());
    for (const auto& source_column : source.columns) {
        if (source_column == "template_offset" ||
            source_column == "list_offset" ||
            source_column == "position_offset") {
            continue;
        }
        const auto index = semantic.columns.size();
        if (!semantic.column_indexes.emplace(source_column, index).second) {
            failure = path.string() +
                ": candidate semantic columns contain a duplicate name";
            return false;
        }
        semantic.columns.push_back(source_column);
        if (source_column == "actor_name") {
            const auto profile_index = semantic.columns.size();
            semantic.column_indexes.emplace("profile_id", profile_index);
            semantic.columns.emplace_back("profile_id");
        }
    }

    semantic.rows.reserve(source.rows.size());
    for (std::size_t row_index = 0U; row_index < source.rows.size();
         ++row_index) {
        const auto& source_row = source.rows[row_index];
        lua_Integer template_offset = 0;
        lua_Integer entity_kind = 0;
        lua_Integer palette_line = 0;
        if (template_column->second >= source_row.size() ||
            entity_column->second >= source_row.size() ||
            palette_column->second >= source_row.size() ||
            !parse_integer(source_row[template_column->second],
                           template_offset) ||
            !parse_integer(source_row[entity_column->second], entity_kind) ||
            !parse_integer(source_row[palette_column->second], palette_line)) {
            failure = path.string() + ": row " +
                std::to_string(row_index + 2U) +
                " has an invalid actor profile key";
            return false;
        }
        const auto profile = profile_ids.find(
            ActorProfileKey{template_offset, entity_kind, palette_line});
        if (profile == profile_ids.end()) {
            failure = path.string() + ": row " +
                std::to_string(row_index + 2U) +
                " has no extracted semantic actor profile";
            return false;
        }

        std::vector<std::string> runtime_row;
        runtime_row.reserve(semantic.columns.size());
        for (std::size_t column = 0U; column < source.columns.size();
             ++column) {
            const auto& source_column = source.columns[column];
            if (source_column == "template_offset" ||
                source_column == "list_offset" ||
                source_column == "position_offset") {
                continue;
            }
            runtime_row.push_back(source_row[column]);
            if (source_column == "actor_name") {
                runtime_row.push_back(profile->second);
            }
        }
        semantic.rows.push_back(std::move(runtime_row));
    }
    return true;
}

bool make_semantic_alarm_profiles(
    const TsvDocument& source, const std::filesystem::path& path,
    const ActivityListIds& activity_ids, TsvDocument& semantic,
    std::string& failure) {
    semantic.columns = {
        "scene_id", "reinforcement_activity_id", "region_count"};
    for (std::size_t column = 0U; column < semantic.columns.size(); ++column) {
        semantic.column_indexes.emplace(semantic.columns[column], column);
    }
    semantic.rows.reserve(source.rows.size());
    for (std::size_t index = 0U; index < source.rows.size(); ++index) {
        const auto& row = source.rows[index];
        lua_Integer scene_id = 0;
        lua_Integer list_offset = 0;
        lua_Integer region_count = 0;
        if (!row_integer(source, row, "scene_id", scene_id, path,
                         index + 2U, failure) ||
            !row_integer(source, row, "reinforcement_list_address",
                         list_offset, path, index + 2U, failure) ||
            !row_integer(source, row, "region_count", region_count, path,
                         index + 2U, failure) ||
            scene_id < 0 || scene_id > srx::v(srx::Scene::LAST_ID) ||
            list_offset < 0 || list_offset > 0x1FFFFF ||
            region_count < 0 || region_count > 256) {
            if (failure.empty()) {
                failure = path.string() + ": row " +
                    std::to_string(index + 2U) +
                    " has invalid alarm profile fields";
            }
            return false;
        }
        std::string activity_id;
        if (list_offset != 0) {
            const auto found = activity_ids.find({scene_id, list_offset});
            if (found == activity_ids.end()) {
                failure = path.string() + ": row " +
                    std::to_string(index + 2U) +
                    " alarm list has no extracted activity id";
                return false;
            }
            activity_id = found->second;
        }
        semantic.rows.push_back({
            std::to_string(scene_id), std::move(activity_id),
            std::to_string(region_count)});
    }
    return true;
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
    lua_createtable(state, 0, 12);
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
    lua_createtable(state, 0, 0);
    lua_setfield(state, root_table, "world_activity_outcomes");
    lua_createtable(state, 0, 0);
    lua_setfield(state, root_table, "dynamic_world_activity_actors");
    lua_createtable(state, 0, 0);
    lua_setfield(state, root_table, "world_activity_actor_lists");
    lua_createtable(state, 0, 0);
    lua_setfield(state, root_table, "conditional_actor_candidates");
    lua_createtable(state, 0, 0);
    lua_setfield(state, root_table, "field_alarm_profiles");
    lua_createtable(state, 0, 0);
    lua_setfield(state, root_table, "field_alarm_regions");
    lua_rawgeti(state, LUA_REGISTRYINDEX, modules_reference);
    lua_pushvalue(state, root_table);
    lua_setfield(state, -2, "world_content_data");
    lua_pop(state, 2);
}

} // namespace

bool load_world_content_tsv(lua_State* state, const int modules_reference,
                            const std::filesystem::path& world_directory,
                            std::string& failure,
                            const std::filesystem::path& field_asset_directory) {
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
    lua_createtable(state, 0, 12);
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
    if (!field_asset_directory.empty()) {
        struct SourceTable {
            const char* relative_path;
            const char* field;
            std::vector<std::string_view> numeric_columns;
            std::vector<std::string_view> optional_numeric_columns;
            std::vector<std::string_view> boolean_columns{};
        };
        const std::array<SourceTable, 4> source_tables{{
            {"world-activity-outcomes.tsv", "world_activity_outcomes",
             {"scene", "context_group", "random_sum"},
             {"handler_offset"}},
            {"entities/dynamic-world-activity-actors.tsv",
             "dynamic_world_activity_actors",
             {"scene", "context_group", "random_sum", "source_order",
               "action_code", "source_spawn_state_flags",
              "source_interaction_flags", "entity_kind",
                "palette_line", "physical_meter",
              "delay_primary_attribute", "delay_secondary_attribute",
              "delay_capability_flags", "combat_mental_meter",
              "combat_cyberware_flags", "combat_posture",
              "combat_body", "combat_quickness", "combat_strength",
              "combat_intelligence", "combat_willpower",
              "combat_melee_combat", "combat_sorcery",
              "combat_armor_index",
              "combat_race_or_state", "combat_class_code",
              "combat_modifier_flags_df", "combat_profile_index_a0",
              "combat_item_code_0", "combat_item_code_1",
              "combat_item_code_2", "combat_item_code_3",
              "combat_item_code_4", "combat_item_code_5",
              "combat_item_code_6", "combat_item_code_7",
              "combat_item_phase_0", "combat_item_phase_1",
              "combat_item_phase_2", "combat_item_phase_3",
              "combat_item_phase_4", "combat_item_phase_5",
              "combat_item_phase_6", "combat_item_phase_7",
              "combat_action_phase_76", "combat_action_phase_77",
              "combat_action_phase_78", "combat_action_phase_79",
              "combat_action_phase_7a", "combat_action_phase_7b",
              "combat_action_phase_7c", "combat_action_phase_7d",
              "combat_action_phase_7e", "combat_action_phase_7f",
              "combat_firearms", "combat_pistols", "combat_smgs",
              "combat_shotguns",
              "combat_auxiliary_roll_bonus", "source_action_slot",
              "source_action_ammo", "source_action_counter",
              "run_target_area", "run_target_event", "run_target_x",
              "run_target_y"},
             {},
             {"activity_probe_enabled"}},
            {"entities/world-activity-actors.tsv",
             "world_activity_actor_lists",
             {"scene", "source_order", "script_index", "template_offset",
              "entity_kind", "action_code",
              "source_action_slot", "source_action_ammo",
              "source_action_counter", "palette_line", "world_x",
              "world_y", "initial_heading", "source_spawn_state_flags",
              "world_effect_active_gate", "pending_world_effect_gate",
              "source_interaction_flags", "physical_meter",
              "delay_primary_attribute", "delay_secondary_attribute",
              "delay_capability_flags", "combat_mental_meter",
              "combat_cyberware_flags", "combat_posture",
              "combat_body", "combat_quickness", "combat_strength",
              "combat_intelligence", "combat_willpower",
              "combat_melee_combat", "combat_sorcery",
              "combat_armor_index",
              "combat_race_or_state", "combat_class_code",
              "combat_modifier_flags_df", "combat_profile_index_a0",
              "combat_item_code_0", "combat_item_code_1",
              "combat_item_code_2", "combat_item_code_3",
              "combat_item_code_4", "combat_item_code_5",
              "combat_item_code_6", "combat_item_code_7",
              "combat_item_phase_0", "combat_item_phase_1",
              "combat_item_phase_2", "combat_item_phase_3",
              "combat_item_phase_4", "combat_item_phase_5",
              "combat_item_phase_6", "combat_item_phase_7",
              "combat_action_phase_76", "combat_action_phase_77",
              "combat_action_phase_78", "combat_action_phase_79",
              "combat_action_phase_7a", "combat_action_phase_7b",
              "combat_action_phase_7c", "combat_action_phase_7d",
              "combat_action_phase_7e", "combat_action_phase_7f",
              "combat_firearms", "combat_pistols", "combat_smgs",
              "combat_shotguns",
              "combat_auxiliary_roll_bonus"}, {},
             {"activity_probe_enabled"}},
            {"entities/conditional-actor-candidates.tsv",
             "conditional_actor_candidates",
             {"scene", "choice", "entity_kind", "palette_line",
               "world_x", "world_y", "heading_word", "direction",
              "source_spawn_state_flags"}},
        }};
        ActivityListIds activity_ids;
        ActorProfileIds actor_profile_ids;
        SourceSequenceIds source_sequence_ids;
        if (!load_source_sequence_ids(world_directory, source_sequence_ids,
                                      failure)) {
            lua_settop(state, stack_base);
            return false;
        }
        for (const auto& source_table : source_tables) {
            TsvDocument document;
            const auto path = field_asset_directory / source_table.relative_path;
            if (!read_tsv(path, false, document, failure)) {
                lua_settop(state, stack_base);
                return false;
            }
            TsvDocument semantic_actor_rows;
            TsvDocument semantic_dynamic_actor_rows;
            TsvDocument semantic_candidate_rows;
            const TsvDocument* runtime_document = &document;
            if (std::string_view(source_table.field) ==
                "dynamic_world_activity_actors") {
                if (!make_semantic_dynamic_actor_rows(
                        document, path, source_sequence_ids,
                        semantic_dynamic_actor_rows, failure)) {
                    lua_settop(state, stack_base);
                    return false;
                }
                runtime_document = &semantic_dynamic_actor_rows;
            } else if (std::string_view(source_table.field) ==
                "world_activity_actor_lists") {
                if (!collect_activity_list_ids(document, path, activity_ids,
                                               failure) ||
                    !collect_actor_profile_ids(document, path,
                                               actor_profile_ids, failure) ||
                    !make_semantic_activity_actor_rows(
                        document, path, semantic_actor_rows, failure)) {
                    lua_settop(state, stack_base);
                    return false;
                }
                runtime_document = &semantic_actor_rows;
            } else if (std::string_view(source_table.field) ==
                       "conditional_actor_candidates") {
                if (!make_semantic_conditional_actor_rows(
                        document, path, actor_profile_ids,
                        semantic_candidate_rows, failure)) {
                    lua_settop(state, stack_base);
                    return false;
                }
                runtime_document = &semantic_candidate_rows;
            }
            if (!push_rows(state, *runtime_document,
                           source_table.numeric_columns, path, failure, {},
                           source_table.optional_numeric_columns,
                           source_table.boolean_columns)) {
                lua_settop(state, stack_base);
                return false;
            }
            lua_setfield(state, root_table, source_table.field);
        }

        const auto profile_path =
            field_asset_directory / "field-alarm-profiles.tsv";
        TsvDocument alarm_profiles;
        TsvDocument semantic_alarm_profiles;
        // TSV pointers are extraction provenance. Resolve the reinforcement
        // list against the extracted actor catalog here, before Lua receives
        // the scene's semantic activity ID and region count.
        if (!read_tsv(profile_path, true, alarm_profiles, failure) ||
            !make_semantic_alarm_profiles(
                alarm_profiles, profile_path, activity_ids,
                semantic_alarm_profiles, failure) ||
            !push_scene_indexed_rows(
                state, semantic_alarm_profiles,
                {"scene_id", "region_count"},
                profile_path, failure, false)) {
            lua_settop(state, stack_base);
            return false;
        }
        lua_setfield(state, root_table, "field_alarm_profiles");

        const auto regions_path =
            field_asset_directory / "field-alarm-regions.tsv";
        TsvDocument alarm_regions;
        if (!read_tsv(regions_path, true, alarm_regions, failure) ||
            !push_scene_indexed_rows(
                state, alarm_regions,
                {"scene_id", "x_min", "x_max", "y_min", "y_max"},
                regions_path, failure, true)) {
            lua_settop(state, stack_base);
            return false;
        }
        lua_setfield(state, root_table, "field_alarm_regions");
    } else {
        lua_createtable(state, 0, 0);
        lua_setfield(state, root_table, "world_activity_outcomes");
        lua_createtable(state, 0, 0);
        lua_setfield(state, root_table, "dynamic_world_activity_actors");
        lua_createtable(state, 0, 0);
        lua_setfield(state, root_table, "world_activity_actor_lists");
        lua_createtable(state, 0, 0);
        lua_setfield(state, root_table, "field_alarm_profiles");
        lua_createtable(state, 0, 0);
        lua_setfield(state, root_table, "field_alarm_regions");
    }
    lua_rawgeti(state, LUA_REGISTRYINDEX, modules_reference);
    lua_pushvalue(state, root_table);
    lua_setfield(state, -2, "world_content_data");
    lua_settop(state, stack_base);
    return true;
}

} // namespace shadowrun::ghidra_conversion::scripting::detail
