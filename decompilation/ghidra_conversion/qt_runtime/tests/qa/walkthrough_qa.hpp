#pragma once

#include "../../src/field/field_scene_runtime.hpp"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime::qa {
namespace walkthrough_detail {

constexpr std::uint32_t kFrameBudget = 3600U;
constexpr std::uint32_t kStoryMilestoneCount = 33U;
constexpr std::uint16_t kSourceRandomSeed = 23191U;

struct HostContext {
    FieldSceneRuntime* runtime = nullptr;
    const std::vector<std::string>* source_events = nullptr;
    std::size_t source_event_start = 0U;
    std::uint32_t frames_this_step = 0U;
    std::string step_id;
    std::string host_error;
    std::ostringstream captured_stderr;
    std::streambuf* previous_stderr = nullptr;
    std::string pending_stderr;
    bool first_step = true;
};

struct LuaModuleRoots {
    std::array<std::filesystem::path, 2> roots;
};

inline int require_script_module(lua_State* state) {
    std::size_t name_length = 0U;
    const auto* module_name = luaL_checklstring(state, 1, &name_length);
    const std::string name(module_name, name_length);

    lua_getglobal(state, "package");
    lua_getfield(state, -1, "loaded");
    lua_pushlstring(state, name.data(), name.size());
    lua_gettable(state, -2);
    if (!lua_isnil(state, -1)) {
        lua_pushvalue(state, -1);
        lua_replace(state, 1);
        lua_settop(state, 1);
        return 1;
    }
    lua_settop(state, 0);

    std::string relative_path = name;
    std::replace(relative_path.begin(), relative_path.end(), '.', '/');
    relative_path += ".lua";
    const auto* roots = static_cast<const LuaModuleRoots*>(
        lua_touserdata(state, lua_upvalueindex(1)));
    std::filesystem::path script_path;
    for (const auto& root : roots->roots) {
        const auto candidate = root / relative_path;
        std::error_code error;
        if (std::filesystem::is_regular_file(candidate, error) && !error) {
            script_path = candidate;
            break;
        }
    }
    if (script_path.empty()) {
        lua_pushfstring(state, "module '%s' not found", name.c_str());
        return lua_error(state);
    }
    if (luaL_loadfilex(state, script_path.string().c_str(), nullptr) != LUA_OK ||
        lua_pcall(state, 0, 1, 0) != LUA_OK) {
        return lua_error(state);
    }
    if (lua_isnil(state, -1)) {
        lua_pop(state, 1);
        lua_pushboolean(state, 1);
    }
    const auto result_index = lua_gettop(state);
    lua_getglobal(state, "package");
    lua_getfield(state, -1, "loaded");
    lua_pushlstring(state, name.data(), name.size());
    lua_pushvalue(state, result_index);
    lua_settable(state, -3);
    lua_pop(state, 2);
    return 1;
}

inline HostContext* context(lua_State* state) {
    return static_cast<HostContext*>(
        lua_touserdata(state, lua_upvalueindex(1)));
}

inline bool integer_argument(lua_State* state, const int index,
                             lua_Integer& value) {
    int is_number = 0;
    value = lua_tointegerx(state, index, &is_number);
    return is_number != 0;
}

inline bool advance_one(HostContext& host, const std::uint8_t input) {
    if (host.frames_this_step >= kFrameBudget) {
        host.host_error = "step exceeded its 3600-frame budget";
        return false;
    }
    host.runtime->set_controller_port_byte(input);
    host.runtime->advance_frame();
    ++host.frames_this_step;
    return true;
}

inline std::optional<std::uint8_t> pad_byte(const std::string_view name) {
    const auto byte = [](const srx::Pad pad) {
        return static_cast<std::uint8_t>(
            static_cast<std::uint16_t>(pad) >> 8U);
    };
    if (name == "UP") return byte(srx::Pad::UP);
    if (name == "DOWN") return byte(srx::Pad::DOWN);
    if (name == "LEFT") return byte(srx::Pad::LEFT);
    if (name == "RIGHT") return byte(srx::Pad::RIGHT);
    if (name == "A") return byte(srx::Pad::A);
    if (name == "B") return byte(srx::Pad::B);
    if (name == "C") return byte(srx::Pad::C);
    if (name == "START") return byte(srx::Pad::START);
    if (name == "NEUTRAL") return 0U;
    return std::nullopt;
}

inline int begin_step(lua_State* state) {
    auto* host = context(state);
    std::size_t length = 0U;
    const auto* id = luaL_checklstring(state, 1, &length);
    host->step_id.assign(id, length);
    host->frames_this_step = 0U;
    host->host_error.clear();
    host->source_event_start = host->first_step
        ? 0U : host->source_events->size();
    host->first_step = false;
    host->captured_stderr.str(host->pending_stderr);
    host->pending_stderr.clear();
    host->captured_stderr.clear();
    host->previous_stderr = std::cerr.rdbuf(host->captured_stderr.rdbuf());
    return 0;
}

inline bool is_boundary_marker(const std::string_view event) {
    return event.find("boundary") != std::string_view::npos ||
           event.find("unresolved") != std::string_view::npos ||
           event.find("unhandled") != std::string_view::npos ||
           event.find("failed") != std::string_view::npos;
}

inline int end_step(lua_State* state) {
    auto* host = context(state);
    if (host->previous_stderr != nullptr) {
        std::cerr.rdbuf(host->previous_stderr);
        host->previous_stderr = nullptr;
    }
    lua_createtable(state, 0, 5);
    const auto captured_stderr = host->captured_stderr.str();
    lua_pushlstring(state, captured_stderr.data(), captured_stderr.size());
    lua_setfield(state, -2, "stderr");
    lua_pushinteger(state, host->frames_this_step);
    lua_setfield(state, -2, "frames");
    lua_pushlstring(state, host->host_error.data(), host->host_error.size());
    lua_setfield(state, -2, "host_error");

    lua_createtable(state, 0, 0);
    lua_Integer event_index = 1;
    lua_Integer boundary_count = 0;
    for (std::size_t index = host->source_event_start;
         index < host->source_events->size(); ++index) {
        const auto& event = (*host->source_events)[index];
        lua_pushlstring(state, event.data(), event.size());
        lua_rawseti(state, -2, event_index++);
        if (is_boundary_marker(event)) {
            ++boundary_count;
        }
    }
    lua_setfield(state, -2, "source_events");
    lua_pushinteger(state, boundary_count);
    lua_setfield(state, -2, "source_boundary_count");
    return 1;
}

inline int press(lua_State* state) {
    auto* host = context(state);
    std::size_t button_length = 0U;
    const auto* button_text = luaL_checklstring(state, 1, &button_length);
    lua_Integer hold_frames = 1;
    lua_Integer settle_frames = 14;
    const auto button = pad_byte(std::string_view(button_text, button_length));
    if (!button.has_value() ||
        (lua_gettop(state) >= 2 &&
         (!integer_argument(state, 2, hold_frames) || hold_frames < 1 ||
          hold_frames > 120)) ||
        (lua_gettop(state) >= 3 &&
         (!integer_argument(state, 3, settle_frames) || settle_frames < 0 ||
          settle_frames > 240))) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "invalid controller input arguments");
        return 2;
    }
    host->runtime->set_controller_port_byte(0U);
    for (lua_Integer frame = 0;
         frame < 180 && host->runtime->dialogue_input_locked(); ++frame) {
        if (!advance_one(*host, 0U)) {
            lua_pushboolean(state, 0);
            lua_pushlstring(state, host->host_error.data(),
                            host->host_error.size());
            return 2;
        }
    }
    if (host->runtime->dialogue_input_locked()) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "dialogue input lockout did not elapse");
        return 2;
    }
    for (lua_Integer frame = 0; frame < hold_frames; ++frame) {
        if (!advance_one(*host, *button)) {
            lua_pushboolean(state, 0);
            lua_pushlstring(state, host->host_error.data(),
                            host->host_error.size());
            return 2;
        }
    }
    for (lua_Integer frame = 0; frame <= settle_frames; ++frame) {
        if (!advance_one(*host, 0U)) {
            lua_pushboolean(state, 0);
            lua_pushlstring(state, host->host_error.data(),
                            host->host_error.size());
            return 2;
        }
    }
    lua_pushboolean(state, 1);
    return 1;
}

inline int hold_until_modal(lua_State* state) {
    auto* host = context(state);
    std::size_t button_length = 0U;
    const auto* button_text = luaL_checklstring(state, 1, &button_length);
    lua_Integer limit = 0;
    const auto button = pad_byte(std::string_view(button_text, button_length));
    if (!button.has_value() || *button == 0U ||
        !integer_argument(state, 2, limit) || limit < 1 ||
        limit > static_cast<lua_Integer>(kFrameBudget)) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "invalid movement-to-modal arguments");
        return 2;
    }
    auto& runtime = *host->runtime;
    for (lua_Integer frame = 0; frame < limit &&
                               !runtime.dialogue_open() &&
                               !runtime.field_interaction_open() &&
                               !runtime.taxi_selector_open_for_dump();
         ++frame) {
        if (!advance_one(*host, *button) || !advance_one(*host, 0U)) {
            lua_pushboolean(state, 0);
            lua_pushlstring(state, host->host_error.data(),
                            host->host_error.size());
            return 2;
        }
    }
    lua_pushboolean(state, runtime.dialogue_open() ||
                               runtime.field_interaction_open() ||
                               runtime.taxi_selector_open_for_dump());
    return 1;
}

inline int advance_frames(lua_State* state) {
    auto* host = context(state);
    lua_Integer frames = 0;
    if (!integer_argument(state, 1, frames) || frames < 0 ||
        frames > static_cast<lua_Integer>(kFrameBudget)) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "invalid neutral frame count");
        return 2;
    }
    for (lua_Integer frame = 0; frame < frames; ++frame) {
        if (!advance_one(*host, 0U)) {
            lua_pushboolean(state, 0);
            lua_pushlstring(state, host->host_error.data(),
                            host->host_error.size());
            return 2;
        }
    }
    lua_pushboolean(state, 1);
    return 1;
}

inline int wait_for_dialogue(lua_State* state) {
    auto* host = context(state);
    lua_Integer limit = 0;
    if (!integer_argument(state, 1, limit) || limit < 0 ||
        limit > static_cast<lua_Integer>(kFrameBudget)) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "invalid dialogue wait frame count");
        return 2;
    }
    for (lua_Integer frame = 0;
         frame <= limit && !host->runtime->dialogue_open(); ++frame) {
        if (frame == limit || !advance_one(*host, 0U)) {
            break;
        }
    }
    lua_pushboolean(state, host->runtime->dialogue_open());
    return 1;
}

inline int wait_for_page(lua_State* state) {
    auto* host = context(state);
    std::size_t key_length = 0U;
    const auto* key = luaL_checklstring(state, 1, &key_length);
    lua_Integer limit = 0;
    if (!integer_argument(state, 2, limit) || limit < 0 ||
        limit > static_cast<lua_Integer>(kFrameBudget)) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "invalid page wait frame count");
        return 2;
    }
    const auto expected = std::string_view(key, key_length);
    for (lua_Integer frame = 0;
         frame < limit &&
         host->runtime->active_story_page_text_key_for_dump() != expected;
         ++frame) {
        if (!advance_one(*host, 0U)) break;
    }
    lua_pushboolean(state,
        host->runtime->active_story_page_text_key_for_dump() == expected);
    return 1;
}

inline int wait_for_taxi_selector(lua_State* state) {
    auto* host = context(state);
    lua_Integer limit = 0;
    if (!integer_argument(state, 1, limit) || limit < 0 ||
        limit > static_cast<lua_Integer>(kFrameBudget)) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "invalid taxi selector wait frame count");
        return 2;
    }
    for (lua_Integer frame = 0;
         frame <= limit && !host->runtime->taxi_selector_open_for_dump();
         ++frame) {
        if (frame == limit || !advance_one(*host, 0U)) {
            break;
        }
    }
    lua_pushboolean(state, host->runtime->taxi_selector_open_for_dump());
    return 1;
}

inline int wait_for_scene(lua_State* state) {
    auto* host = context(state);
    lua_Integer scene = 0;
    lua_Integer limit = 0;
    if (!integer_argument(state, 1, scene) || scene < 0 || scene > 65535 ||
        !integer_argument(state, 2, limit) || limit < 0 ||
        limit > static_cast<lua_Integer>(kFrameBudget)) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "invalid scene wait arguments");
        return 2;
    }
    for (lua_Integer frame = 0;
         frame <= limit &&
         host->runtime->field_scene_reference_for_dump() !=
             static_cast<std::uint16_t>(scene);
         ++frame) {
        if (frame == limit || !advance_one(*host, 0U)) {
            break;
        }
    }
    const bool reached = host->runtime->field_scene_reference_for_dump() ==
                         static_cast<std::uint16_t>(scene);
    lua_pushboolean(state, reached);
    return 1;
}

inline int place_within_scene(lua_State* state) {
    auto* host = context(state);
    lua_Integer scene = 0;
    lua_Integer x = 0;
    lua_Integer y = 0;
    if (!integer_argument(state, 1, scene) || scene < 0 || scene > 65535 ||
        !integer_argument(state, 2, x) || x < 0 || x > 65535 ||
        !integer_argument(state, 3, y) || y < 0 || y > 65535) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "invalid within-scene placement arguments");
        return 2;
    }
    const auto actual_scene = host->runtime->field_scene_reference_for_dump();
    if (actual_scene != static_cast<std::uint16_t>(scene)) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state,
                        "within-scene placement refused a different scene");
        return 2;
    }
    host->runtime->set_field_entity_world_position_for_dump(
        static_cast<std::uint16_t>(x), static_cast<std::uint16_t>(y));
    const auto camera_x = static_cast<std::uint16_t>(
        host->runtime->field_entity_world_position().x > 4096U
            ? host->runtime->field_entity_world_position().x - 4096U
            : 0U);
    const auto camera_y = static_cast<std::uint16_t>(
        host->runtime->field_entity_world_position().y > 3072U
            ? host->runtime->field_entity_world_position().y - 3072U
            : 0U);
    host->runtime->set_field_camera_world_position_for_dump(camera_x, camera_y);
    const bool still_same_scene =
        host->runtime->field_scene_reference_for_dump() == actual_scene;
    lua_pushboolean(state, still_same_scene);
    if (!still_same_scene) {
        lua_pushliteral(state, "within-scene placement changed scene");
        return 2;
    }
    return 1;
}

inline void set_optional_integer(lua_State* state, const char* name,
                                 const std::optional<std::uint32_t> value) {
    if (value.has_value()) {
        lua_pushinteger(state, *value);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, name);
}

inline int snapshot(lua_State* state) {
    auto* host = context(state);
    auto& runtime = *host->runtime;
    lua_createtable(state, 0, 24);
    lua_pushinteger(state, runtime.field_world_fact_flags_for_dump());
    lua_setfield(state, -2, "story");
    lua_pushinteger(state, runtime.field_mode_flags_for_dump());
    lua_setfield(state, -2, "mode");
    lua_pushinteger(state, runtime.field_mode_effect_flags_for_dump());
    lua_setfield(state, -2, "effect");
    lua_pushinteger(state, runtime.field_auxiliary_flags_for_dump());
    lua_setfield(state, -2, "auxiliary");
    set_optional_integer(state, "group_item",
                         runtime.group_item_flags_for_dump());
    const auto dialogue_flags = runtime.field_dialogue_flags_for_dump();
    if (dialogue_flags.has_value()) {
        lua_pushinteger(state, *dialogue_flags);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "dialogue");
    lua_pushboolean(state, runtime.field_interaction_open());
    lua_setfield(state, -2, "interaction_open");
    set_optional_integer(state, "nuyen", runtime.field_funds_for_dump());
    lua_pushinteger(state, runtime.field_scene_reference_for_dump());
    lua_setfield(state, -2, "scene");
    lua_pushboolean(state, runtime.dialogue_open());
    lua_setfield(state, -2, "dialogue_open");
    lua_pushboolean(state, runtime.command_overview_open());
    lua_setfield(state, -2, "command_open");
    lua_pushboolean(state, runtime.taxi_selector_open_for_dump());
    lua_setfield(state, -2, "taxi_selector_open");
    const auto page_key = runtime.active_story_page_text_key_for_dump();
    lua_pushlstring(state, page_key.data(), page_key.size());
    lua_setfield(state, -2, "page_key");
    const auto& dialogue_panel = runtime.dialogue_panel();
    lua_pushboolean(state, dialogue_panel.open);
    lua_setfield(state, -2, "panel_open");
    lua_pushlstring(state, dialogue_panel.actor_name.data(),
                    dialogue_panel.actor_name.size());
    lua_setfield(state, -2, "panel_actor");
    lua_pushinteger(state, static_cast<lua_Integer>(dialogue_panel.page_index));
    lua_setfield(state, -2, "panel_page");
    std::string panel_text;
    for (std::size_t index = 0U;
         index < dialogue_panel.text_segment_count; ++index) {
        if (!panel_text.empty()) panel_text.push_back(' ');
        panel_text.append(dialogue_panel.text_segments[index]);
    }
    lua_pushlstring(state, panel_text.data(), panel_text.size());
    lua_setfield(state, -2, "panel_text");
    const auto& position = runtime.field_entity_world_position();
    lua_pushinteger(state, position.x);
    lua_setfield(state, -2, "x");
    lua_pushinteger(state, position.y);
    lua_setfield(state, -2, "y");
    lua_pushboolean(state, runtime.opening_auto_walk_active_for_dump());
    lua_setfield(state, -2, "opening_auto_walk");
    lua_pushinteger(state, runtime.field_mode_event_code_for_dump());
    lua_setfield(state, -2, "event");
    const auto karma = runtime.party_karma_for_dump();
    if (karma.has_value()) {
        lua_createtable(state, 3, 0);
        for (std::size_t index = 0U; index < karma->size(); ++index) {
            lua_pushinteger(state, (*karma)[index]);
            lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
        }
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "karma");
    const auto runner_features = runtime.field_runner_feature_flags_for_dump();
    if (runner_features.has_value()) {
        lua_pushinteger(state, *runner_features);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "runner_features");

    lua_createtable(state, 0, 3);
    const auto story_flags = runtime.field_world_fact_flags_for_dump();
    const std::array<std::pair<const char*, srx::StoryFlag>, 3> facts{{
        {"motel_bill_paid", srx::StoryFlag::MOTEL_BILL_PAID},
        {"tarislar_asked_shale", srx::StoryFlag::TARISLAR_ASKED_SHALE},
        {"michael_alias_learned", srx::StoryFlag::MICHAEL_ALIAS_LEARNED},
    }};
    for (const auto& [name, flag] : facts) {
        const auto bit = static_cast<std::uint8_t>(flag);
        lua_pushboolean(state,
            (story_flags & (std::uint32_t{1U} << bit)) != 0U);
        lua_setfield(state, -2, name);
    }
    lua_setfield(state, -2, "story_facts");

    lua_createtable(state, 0, 3);
    for (const auto& [name, flag] : facts) {
        lua_pushinteger(state, static_cast<std::uint8_t>(flag));
        lua_setfield(state, -2, name);
    }
    lua_setfield(state, -2, "story_fact_bits");

    QString snapshot_error;
    const auto save_snapshot = runtime.capture_game_save_snapshot_for_dump(
        &snapshot_error);
    QJsonObject item_fields;
    if (save_snapshot.has_value()) {
        std::function<void(const QJsonValue&, const QString&, bool)> collect =
            [&](const QJsonValue& value, const QString& path, const bool keep) {
                if (keep) item_fields.insert(path, value);
                if (value.isObject()) {
                    const auto object = value.toObject();
                    for (auto iterator = object.begin();
                         iterator != object.end(); ++iterator) {
                        const auto key = iterator.key().toLower();
                        collect(iterator.value(), path + "/" + iterator.key(),
                            key.contains("item") || key.contains("inventor") ||
                                key.contains("equip"));
                    }
                } else if (value.isArray()) {
                    const auto array = value.toArray();
                    for (qsizetype index = 0; index < array.size(); ++index) {
                        collect(array[index], path + "/" +
                            QString::number(index), false);
                    }
                }
            };
        collect(QJsonValue(*save_snapshot), QString{}, false);
    }
    const auto item_signature = QJsonDocument(item_fields).toJson(
        QJsonDocument::Compact);
    lua_pushlstring(state, item_signature.constData(),
                    static_cast<std::size_t>(item_signature.size()));
    lua_setfield(state, -2, "item_signature");
    return 1;
}

inline void add_host_function(lua_State* state, HostContext& host,
                              const char* name, lua_CFunction function) {
    lua_pushlightuserdata(state, &host);
    lua_pushcclosure(state, function, 1);
    lua_setfield(state, -2, name);
}

inline std::string lua_string_field(lua_State* state, const int table,
                                    const char* field) {
    lua_getfield(state, table, field);
    std::size_t length = 0U;
    const auto* value = lua_tolstring(state, -1, &length);
    std::string result;
    if (value != nullptr) {
        result.assign(value, length);
    }
    lua_pop(state, 1);
    return result;
}

inline bool lua_boolean_field(lua_State* state, const int table,
                              const char* field) {
    lua_getfield(state, table, field);
    const bool value = lua_toboolean(state, -1) != 0;
    lua_pop(state, 1);
    return value;
}

inline lua_Integer lua_integer_field(lua_State* state, const int table,
                                     const char* field) {
    lua_getfield(state, table, field);
    const auto value = lua_tointeger(state, -1);
    lua_pop(state, 1);
    return value;
}

inline std::string clean_tsv(std::string value) {
    std::replace(value.begin(), value.end(), '\t', ' ');
    std::replace(value.begin(), value.end(), '\r', ' ');
    std::replace(value.begin(), value.end(), '\n', ' ');
    return value;
}

inline void write_lua_report(lua_State* state,
                             const std::filesystem::path& output_directory,
                             const std::string_view segment,
                             const bool reach_mode,
                             const std::string_view setup_error = {}) {
    std::filesystem::create_directories(output_directory);
    std::ofstream steps(output_directory / "steps.tsv", std::ios::trunc);
    std::ofstream stderr_log(output_directory / "stderr.log", std::ios::trunc);
    std::ofstream reach(output_directory / "walkthrough_reach.tsv",
                        std::ios::trunc);
    steps << "segment\tstep\tmilestone\tresult\tframes\tbefore\tafter\tdetail\n";
    stderr_log << "segment\tstep\tstderr\n";

    std::string last_step = "none";
    std::string first_broken;
    std::string failure = std::string(setup_error);
    lua_Integer last_milestone = 0;
    bool passed = setup_error.empty();
    if (lua_istable(state, -1)) {
        last_step = lua_string_field(state, -1, "last_step");
        first_broken = lua_string_field(state, -1, "first_broken");
        failure = lua_string_field(state, -1, "failure");
        last_milestone = lua_integer_field(state, -1, "last_milestone");
        passed = lua_boolean_field(state, -1, "passed");
        lua_getfield(state, -1, "steps");
        const auto array = lua_gettop(state);
        const auto length = lua_rawlen(state, array);
        for (std::size_t index = 1U; index <= length; ++index) {
            lua_rawgeti(state, array, static_cast<lua_Integer>(index));
            const auto entry = lua_gettop(state);
            const auto id = clean_tsv(lua_string_field(state, entry, "id"));
            const auto detail = clean_tsv(
                lua_string_field(state, entry, "detail"));
            const auto before = clean_tsv(
                lua_string_field(state, entry, "before_summary"));
            const auto after = clean_tsv(
                lua_string_field(state, entry, "after_summary"));
            const auto frames = lua_integer_field(state, entry, "frames");
            const auto milestone = lua_integer_field(state, entry, "milestone");
            const bool step_passed = lua_boolean_field(state, entry, "passed");
            steps << segment << '\t' << id << '\t' << milestone << '\t'
                  << (step_passed ? "PASS" : "FAIL") << '\t' << frames << '\t'
                  << before << '\t' << after << '\t' << detail << '\n';
            const auto step_stderr = lua_string_field(state, entry, "stderr");
            if (!step_stderr.empty()) {
                stderr_log << segment << '\t' << id << '\t'
                           << clean_tsv(step_stderr) << '\n';
            }
            lua_pop(state, 1);
        }
        lua_pop(state, 1);
    }
    steps.flush();
    stderr_log.flush();
    reach << "segment\tstep\tmilestone\tcount\n"
          << segment << '\t' << last_step << '\t' << last_milestone << '\t'
          << kStoryMilestoneCount << '\n';
    reach.flush();

    std::cout << "walkthrough reach: " << segment << '/' << last_step
              << " (milestone " << last_milestone << " of "
              << kStoryMilestoneCount << ")\n";
    if (!passed) {
        std::cerr << "walkthrough " << segment << " stopped at "
                  << (first_broken.empty() ? "bootstrap" : first_broken)
                  << ": " << failure << '\n';
    }
    if (!reach_mode && !passed) {
        std::cerr << "walkthrough gate failed; details: "
                  << (output_directory / "steps.tsv").string() << '\n';
    }
}

} // namespace walkthrough_detail

inline int run_walkthrough_qa(
    shadowrun::ghidra_conversion::integration::StartupIntegrationState
        startup_state,
    const std::filesystem::path& asset_root,
    const std::string& segment,
    const std::filesystem::path& output_directory,
    const bool reach_mode) {
    using namespace walkthrough_detail;
    std::error_code error;
    std::filesystem::create_directories(output_directory, error);
    if (error) {
        std::cerr << "walkthrough: cannot create output directory: "
                  << error.message() << '\n';
        std::cout << "walkthrough reach: " << segment
                  << "/none (milestone 0 of " << kStoryMilestoneCount
                  << ")\n";
        return reach_mode ? 0 : 3;
    }

    std::vector<std::string> source_events;
    std::vector<std::uint16_t> audio_events;
    FieldSceneRuntime runtime(
        [&](const std::string_view event) {
            source_events.emplace_back(event);
        },
        [&](const std::uint16_t event) { audio_events.push_back(event); },
        [](const std::uint16_t) {});
    runtime.set_source_random_seed_for_dump(kSourceRandomSeed);
    startup_state.new_game_opening_completed = true;
    std::ostringstream bootstrap_stderr;
    auto* previous_stderr = std::cerr.rdbuf(bootstrap_stderr.rdbuf());
    if (!runtime.bootstrap(startup_state)) {
        std::cerr.rdbuf(previous_stderr);
        const auto message = "production New Game bootstrap failed: " +
                             runtime.failure();
        std::cerr << "walkthrough: " << message << '\n';
        lua_State* empty_state = luaL_newstate();
        if (empty_state != nullptr) {
            write_lua_report(empty_state, output_directory, segment,
                             reach_mode, message);
            lua_close(empty_state);
        }
        return reach_mode ? 0 : 3;
    }
    std::cerr.rdbuf(previous_stderr);

    lua_State* state = luaL_newstate();
    if (state == nullptr) {
        std::cerr << "walkthrough: could not allocate Lua state\n";
        std::cout << "walkthrough reach: " << segment
                  << "/none (milestone 0 of " << kStoryMilestoneCount
                  << ")\n";
        return reach_mode ? 0 : 3;
    }
    const auto gameplay_root = asset_root.parent_path() / "gameplay";
    const auto scripts_root = gameplay_root / "scripts";
    const auto tests_root = gameplay_root / "tests_lua";
    luaL_requiref(state, LUA_GNAME, luaopen_base, 1);
    lua_pop(state, 1);
    luaL_requiref(state, LUA_TABLIBNAME, luaopen_table, 1);
    lua_pop(state, 1);
    luaL_requiref(state, LUA_STRLIBNAME, luaopen_string, 1);
    lua_pop(state, 1);
    luaL_requiref(state, LUA_MATHLIBNAME, luaopen_math, 1);
    lua_pop(state, 1);
    luaL_requiref(state, LUA_UTF8LIBNAME, luaopen_utf8, 1);
    lua_pop(state, 1);

    lua_newtable(state);
    lua_newtable(state);
    lua_setfield(state, -2, "loaded");
    const auto package_path =
        (scripts_root / "?.lua").string() + ";" +
        (scripts_root / "?" / "init.lua").string() + ";" +
        (tests_root / "?.lua").string() + ";" +
        (tests_root / "?" / "init.lua").string();
    lua_pushlstring(state, package_path.data(), package_path.size());
    lua_setfield(state, -2, "path");
    lua_setglobal(state, "package");
    const LuaModuleRoots module_roots{{scripts_root, tests_root}};
    lua_pushlightuserdata(state, const_cast<LuaModuleRoots*>(&module_roots));
    lua_pushcclosure(state, require_script_module, 1);
    lua_setglobal(state, "require");

    HostContext host{&runtime, &source_events};
    host.pending_stderr = bootstrap_stderr.str();
    lua_getglobal(state, "require");
    lua_pushliteral(state, "walkthrough.driver");
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        const auto* error_text = lua_tostring(state, -1);
        const std::string message = error_text != nullptr
                                        ? error_text
                                        : "could not load walkthrough.driver";
        std::cerr << "walkthrough: " << message << '\n';
        lua_settop(state, 0);
        lua_pushnil(state);
        write_lua_report(state, output_directory, segment, reach_mode,
                         message);
        lua_close(state);
        return reach_mode ? 0 : 3;
    }
    lua_getfield(state, -1, "run");
    lua_remove(state, -2);
    lua_createtable(state, 0, 12);
    add_host_function(state, host, "begin_step", begin_step);
    add_host_function(state, host, "end_step", end_step);
    add_host_function(state, host, "press", press);
    add_host_function(state, host, "hold_until_modal", hold_until_modal);
    add_host_function(state, host, "advance", advance_frames);
    add_host_function(state, host, "wait_for_dialogue", wait_for_dialogue);
    add_host_function(state, host, "wait_for_page", wait_for_page);
    add_host_function(state, host, "wait_for_taxi_selector",
                      wait_for_taxi_selector);
    add_host_function(state, host, "wait_for_scene", wait_for_scene);
    add_host_function(state, host, "place_within_scene", place_within_scene);
    add_host_function(state, host, "snapshot", snapshot);
    const auto output_string = output_directory.string();
    lua_pushlstring(state, output_string.data(), output_string.size());
    lua_setfield(state, -2, "output_directory");
    const auto asset_string = asset_root.string();
    lua_pushlstring(state, asset_string.data(), asset_string.size());
    lua_setfield(state, -2, "asset_root");
    lua_pushlstring(state, segment.data(), segment.size());
    lua_pushboolean(state, reach_mode);
    if (lua_pcall(state, 3, 1, 0) != LUA_OK) {
        const auto* error_text = lua_tostring(state, -1);
        const std::string message = error_text != nullptr
                                        ? error_text
                                        : "walkthrough Lua driver failed";
        std::cerr << "walkthrough: " << message << '\n';
        lua_settop(state, 0);
        lua_pushnil(state);
        write_lua_report(state, output_directory, segment, reach_mode,
                         message);
        lua_close(state);
        return reach_mode ? 0 : 3;
    }

    const bool passed = lua_boolean_field(state, -1, "passed");
    write_lua_report(state, output_directory, segment, reach_mode);
    lua_close(state);
    return passed || reach_mode ? 0 : 1;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime::qa
