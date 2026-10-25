#include "enums.hpp"

#include <QImage>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

using AssetRows = std::unordered_map<std::string, std::filesystem::path>;
using ActorSource = std::tuple<std::uint16_t, std::uint8_t, std::string>;

int require_script_module(lua_State* state) {
    const auto* module_name = luaL_checkstring(state, 1);
    lua_getglobal(state, "package");
    lua_getfield(state, -1, "loaded");
    lua_pushstring(state, module_name);
    lua_gettable(state, -2);
    if (!lua_isnil(state, -1)) {
        return 1;
    }
    lua_pop(state, 3);

    std::string relative_path(module_name);
    std::replace(relative_path.begin(), relative_path.end(), '.', '/');
    relative_path += ".lua";
    const auto* root = lua_tostring(state, lua_upvalueindex(1));
    const auto script_path =
        std::filesystem::path(root == nullptr ? "" : root) / relative_path;
    if (luaL_loadfile(state, script_path.string().c_str()) != LUA_OK ||
        lua_pcall(state, 0, 1, 0) != LUA_OK) {
        return lua_error(state);
    }

    lua_getglobal(state, "package");
    lua_getfield(state, -1, "loaded");
    lua_pushstring(state, module_name);
    lua_pushvalue(state, 2);
    lua_settable(state, -3);
    lua_pop(state, 2);
    return 1;
}

bool split_tsv_line(const std::string& line,
                    std::vector<std::string>& fields) {
    fields.clear();
    std::size_t begin = 0U;
    for (;;) {
        const auto end = line.find('\t', begin);
        fields.push_back(line.substr(
            begin, end == std::string::npos ? std::string::npos
                                             : end - begin));
        if (end == std::string::npos) {
            return true;
        }
        begin = end + 1U;
    }
}

std::unordered_map<std::string, std::size_t> header_columns(
    const std::vector<std::string>& header) {
    std::unordered_map<std::string, std::size_t> result;
    for (std::size_t index = 0U; index < header.size(); ++index) {
        result.emplace(header[index], index);
    }
    return result;
}

bool read_asset_rows(const std::filesystem::path& asset_root,
                     AssetRows& rows) {
    std::ifstream input(asset_root / "resources.tsv", std::ios::binary);
    std::string line;
    std::vector<std::string> fields;
    if (!input || !std::getline(input, line) ||
        line != "alias\tpath\tkind\torigin_x\torigin_y") {
        std::cerr << "actor animation art QA: invalid resources.tsv\n";
        return false;
    }
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        split_tsv_line(line, fields);
        if (fields.size() < 2U ||
            !rows.emplace(fields[0], asset_root / fields[1]).second) {
            std::cerr << "actor animation art QA: malformed or duplicate asset row\n";
            return false;
        }
    }
    return true;
}

bool append_actor_sources(const std::filesystem::path& asset_root,
                          const char* relative_path,
                          std::set<ActorSource>& sources,
                          const AssetRows& rows) {
    std::ifstream input(asset_root / relative_path, std::ios::binary);
    std::string line;
    std::vector<std::string> fields;
    if (!input || !std::getline(input, line)) {
        std::cerr << "actor animation art QA: cannot read " << relative_path
                  << '\n';
        return false;
    }
    split_tsv_line(line, fields);
    const auto columns = header_columns(fields);
    const auto kind_column = columns.find("entity_kind");
    const auto palette_column = columns.find("palette_line");
    const auto stem_column = columns.find("sprite_stem");
    if (kind_column == columns.end() || palette_column == columns.end() ||
        stem_column == columns.end()) {
        std::cerr << "actor animation art QA: actor source table lacks visual keys: "
                  << relative_path << '\n';
        return false;
    }
    while (std::getline(input, line)) {
        if (line.empty()) {
            continue;
        }
        split_tsv_line(line, fields);
        const auto maximum_column = std::max({
            kind_column->second, palette_column->second, stem_column->second});
        if (fields.size() <= maximum_column) {
            std::cerr << "actor animation art QA: malformed actor source row in "
                      << relative_path << '\n';
            return false;
        }
        const auto kind = static_cast<std::uint16_t>(
            std::stoul(fields[kind_column->second]));
        const auto palette = static_cast<std::uint8_t>(
            std::stoul(fields[palette_column->second]));
        const auto& stem = fields[stem_column->second];
        if (kind < 3U || kind > 15U || stem.empty()) {
            continue;
        }
        std::string prefix = "field.entities.actors.";
        for (const char character : stem) {
            prefix.push_back(character == '-' ? '.' : character);
        }
        // The runtime skips source actors whose idle art is not imported.
        if (rows.find(prefix + ".idle") != rows.end()) {
            sources.emplace(kind, palette, stem);
        }
    }
    return true;
}

bool initialize_lua(lua_State* state,
                    const std::filesystem::path& script_root) {
    luaL_requiref(state, LUA_GNAME, luaopen_base, 1);
    lua_pop(state, 1);
    lua_newtable(state);
    lua_newtable(state);
    lua_setfield(state, -2, "loaded");
    const auto module_path = (script_root / "?.lua").string();
    lua_pushlstring(state, module_path.data(), module_path.size());
    lua_setfield(state, -2, "path");
    lua_setglobal(state, "package");
    const auto script_root_string = script_root.string();
    lua_pushlstring(state, script_root_string.data(), script_root_string.size());
    lua_pushcclosure(state, require_script_module, 1);
    lua_setglobal(state, "require");

    constexpr char kLuaHelpers[] =
        "local Animator = require('actor.animator')\n"
        "function __actor_anim_frame_count(kind, pose, direction)\n"
        "  local list = Animator.list(kind, pose, direction)\n"
        "  return list and list.count or -1\n"
        "end\n";
    if (luaL_dostring(state, kLuaHelpers) != LUA_OK) {
        std::cerr << "actor animation art QA: cannot load Lua Animator: "
                  << lua_tostring(state, -1) << '\n';
        return false;
    }
    return true;
}

int lua_frame_count(lua_State* state,
                    const std::uint16_t kind,
                    const srx::ActorPose pose,
                    const std::optional<std::uint8_t> direction) {
    lua_getglobal(state, "__actor_anim_frame_count");
    lua_pushinteger(state, kind);
    lua_pushinteger(state, static_cast<lua_Integer>(pose));
    if (direction.has_value()) {
        lua_pushinteger(state, *direction);
    } else {
        lua_pushnil(state);
    }
    if (lua_pcall(state, 3, 1, 0) != LUA_OK) {
        std::cerr << "actor animation art QA: Lua Animator.list failed: "
                  << lua_tostring(state, -1) << '\n';
        lua_pop(state, 1);
        return -1;
    }
    const auto count = static_cast<int>(lua_tointeger(state, -1));
    lua_pop(state, 1);
    return count;
}

bool verify_loaded_frames(const AssetRows& rows,
                          const std::string& prefix,
                          const std::string& family,
                          const int expected_count) {
    if (expected_count < 1 || expected_count > 16) {
        std::cerr << "actor animation art QA: Animator.list returned an "
                     "unsupported frame count for "
                  << prefix << '.' << family << '\n';
        return false;
    }
    for (int frame = 0; frame < expected_count; ++frame) {
        const auto alias = prefix + "." + family + ".frame." +
                           std::to_string(frame);
        const auto asset = rows.find(alias);
        if (asset == rows.end()) {
            std::cerr << "actor animation art QA: missing frame stem "
                      << alias << '\n';
            return false;
        }
        QImage image;
        if (!image.load(QString::fromStdString(asset->second.string())) ||
            image.isNull()) {
            std::cerr << "actor animation art QA: cannot load "
                      << asset->second.string() << '\n';
            return false;
        }
    }
    const auto next_alias = prefix + "." + family + ".frame." +
                            std::to_string(expected_count);
    if (rows.find(next_alias) != rows.end()) {
        std::cerr << "actor animation art QA: extra frame stem after Lua count: "
                  << next_alias << '\n';
        return false;
    }
    return true;
}

bool verify_spell_frames(lua_State* state,
                         const AssetRows& rows,
                         const std::uint16_t kind,
                         const std::string& prefix) {
    for (std::uint8_t direction = 0U; direction < 8U; ++direction) {
        const auto count = lua_frame_count(
            state, kind, srx::ActorPose::SPELL, direction);
        const auto family = "spell.dir." + std::to_string(direction);
        if (!verify_loaded_frames(rows, prefix, family, count)) {
            return false;
        }
    }
    for (std::uint8_t direction = 0U; direction < 8U; ++direction) {
        auto legacy_alias = prefix + ".spell";
        if (direction != 0U) {
            legacy_alias += ".dir." + std::to_string(direction);
        }
        if (rows.find(legacy_alias) != rows.end()) {
            std::cerr << "actor animation art QA: stale single-image spell alias "
                      << legacy_alias << '\n';
            return false;
        }
    }
    return true;
}

}  // namespace

int main() {
    const std::filesystem::path asset_root(SRX_ACTOR_ANIMATION_ASSET_ROOT);
    const std::filesystem::path script_root(SRX_ACTOR_ANIMATION_SCRIPT_ROOT);
    AssetRows rows;
    if (!read_asset_rows(asset_root, rows)) {
        return 1;
    }
    std::set<ActorSource> actor_sources;
    if (!append_actor_sources(asset_root, "field/entities/default-actors.tsv",
                              actor_sources, rows) ||
        !append_actor_sources(asset_root, "field/entities/world-activity-actors.tsv",
                              actor_sources, rows) ||
        !append_actor_sources(asset_root,
                              "field/entities/dynamic-world-activity-actors.tsv",
                              actor_sources, rows)) {
        return 1;
    }

    lua_State* state = luaL_newstate();
    if (state == nullptr) {
        std::cerr << "actor animation art QA: cannot create Lua state\n";
        return 1;
    }
    luaL_requiref(state, LUA_GNAME, luaopen_base, 1);
    lua_pop(state, 1);
    if (!initialize_lua(state, script_root)) {
        lua_close(state);
        return 1;
    }

    std::set<std::uint16_t> covered_kinds;
    bool passed = !actor_sources.empty();
    for (const auto& [kind, palette, stem] : actor_sources) {
        (void)palette;
        std::string prefix = "field.entities.actors.";
        for (const char character : stem) {
            prefix.push_back(character == '-' ? '.' : character);
        }
        covered_kinds.insert(kind);
        if (!verify_spell_frames(state, rows, kind, prefix) ||
            !verify_loaded_frames(
                rows, prefix, "spell.death.e0",
                lua_frame_count(state, kind,
                                srx::ActorPose::SPELL_DEATH_E0, std::nullopt)) ||
            !verify_loaded_frames(
                rows, prefix, "spell.death.e4",
                lua_frame_count(state, kind,
                                srx::ActorPose::SPELL_DEATH_E4, std::nullopt))) {
            passed = false;
            break;
        }
    }
    for (std::uint16_t kind = 3U; kind <= 15U; ++kind) {
        if (covered_kinds.find(kind) == covered_kinds.end()) {
            std::cerr << "actor animation art QA: no imported actor art source for kind "
                      << kind << '\n';
            passed = false;
        }
    }
    lua_close(state);
    if (!passed) {
        return 1;
    }
    std::cout << "actor animation art QA: loaded spell and spell-death frames "
                 "against Lua Animator.list for "
              << actor_sources.size() << " actor sources\n";
    return 0;
}
