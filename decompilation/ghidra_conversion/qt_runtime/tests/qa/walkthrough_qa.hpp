#pragma once

#include "../../src/field/field_scene_runtime.hpp"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
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
#include <iterator>
#include <limits>
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
constexpr std::string_view kSeg1EndFixtureFormat =
    "shadowrun.walkthrough.save-fixture.v1";
// The checkpoint records the code tip whose save state it captures. Keep this
// stable so regeneration can be compared byte-for-byte after later QA edits.
constexpr std::string_view kSeg1EndFixtureSourceCommit =
    "8000b5cf4dc7c4fc4dd9cbf566c57fc85279af44";
constexpr std::array<std::string_view, 31U> kWalkthroughStoryFactIds{{
    "motel_bill_paid", "tarislar_asked_shale", "owlfeather_asked",
    "aragorn_found", "aragorn_freed_told", "shaman_met",
    "shaman_item4_given", "shaman_pelt_given", "serpent_scale_taken",
    "shaman_tasks_done", "michael_alias_learned", "mako_defeated",
    "renraku_map_overlay", "michael_mission_told", "cyberheart_safe",
    "ogami_left", "ito_fight_won", "ritual_of_binding",
    "red_buffalo_woman", "ilene_met", "gillian_initiation",
    "garden_apts_entered", "landmark_map_taken", "holopix_shown",
    "medicine_lodge_turned_away", "hospital_visited",
    "renraku_map_shown", "both_maps_shown", "post_ito_briefed",
    "heart_delivered", "frosty_met",
}};
// Keep this identity order aligned with kGroupItemFactIds in the runtime's
// fact adapter; the walkthrough host accepts E.GroupItemId values.
constexpr std::array<std::string_view, 15U> kWalkthroughGroupItemIds{{
    "prototype_cyberheart", "indian_lands_visa", "feathered_scale",
    "hell_hound_pelt", "gargoyle_horn", "gift_lady_gillian",
    "gift_red_buffalo_woman", "landmark_map", "map_overlay",
    "current_run_package", "security_pass", "weapon_permit",
    "eye_fiver_ear_cuff", "halloweener_patch", "ork_armband",
}};

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
    bool from_fixture = false;
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

inline int grant_nuyen(lua_State* state) {
    auto* host = context(state);
    lua_Integer amount = 0;
    if (!integer_argument(state, 1, amount) || amount < 0 ||
        amount > std::numeric_limits<std::uint32_t>::max()) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "nuyen grant must be an unsigned 32-bit amount");
        return 2;
    }
    const auto funds = host->runtime->field_funds_for_dump();
    if (!funds.has_value() ||
        static_cast<std::uint64_t>(*funds) +
                static_cast<std::uint64_t>(amount) >
            std::numeric_limits<std::uint32_t>::max()) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "nuyen grant cannot be applied to this runtime");
        return 2;
    }
    host->runtime->set_field_funds_for_dump(
        *funds + static_cast<std::uint32_t>(amount));
    lua_pushboolean(state, 1);
    return 1;
}

inline int grant_karma(lua_State* state) {
    auto* host = context(state);
    if (!lua_istable(state, 1)) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "Karma grant must provide three runner amounts");
        return 2;
    }
    const auto current = host->runtime->party_karma_for_dump();
    if (!current.has_value()) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "party Karma is unavailable for this runtime");
        return 2;
    }
    auto granted = *current;
    for (std::size_t index = 0U; index < granted.size(); ++index) {
        lua_rawgeti(state, 1, static_cast<lua_Integer>(index + 1U));
        lua_Integer amount = 0;
        const bool valid = integer_argument(state, -1, amount) &&
                           amount >= 0 && amount <= 255;
        lua_pop(state, 1);
        if (!valid) {
            lua_pushboolean(state, 0);
            lua_pushliteral(state,
                "each Karma grant must be an unsigned byte amount");
            return 2;
        }
        granted[index] = static_cast<std::uint8_t>(
            (static_cast<unsigned int>(granted[index]) +
             static_cast<unsigned int>(amount)) & 0xFFU);
    }
    host->runtime->seed_party_karma_for_qa(granted);
    lua_pushboolean(state, 1);
    return 1;
}

inline int grant_group_item(lua_State* state) {
    auto* host = context(state);
    std::size_t length = 0U;
    const auto* item_id = luaL_checklstring(state, 1, &length);
    const auto name = std::string_view(item_id, length);
    const auto item = std::find(kWalkthroughGroupItemIds.begin(),
                                kWalkthroughGroupItemIds.end(), name);
    const auto flags = host->runtime->group_item_flags_for_dump();
    if (item == kWalkthroughGroupItemIds.end() || !flags.has_value()) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "unknown group item or unresolved group-item state");
        return 2;
    }
    const auto bit = static_cast<unsigned int>(
        std::distance(kWalkthroughGroupItemIds.begin(), item));
    host->runtime->set_group_item_flags_for_dump(
        *flags | (std::uint32_t{1U} << bit));
    lua_pushboolean(state, 1);
    return 1;
}

inline int set_story_fact_for_setup(lua_State* state) {
    auto* host = context(state);
    lua_Integer bit = 0;
    if (!integer_argument(state, 1, bit) || bit < 0 || bit >= 32) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "story flag bit must be in [0, 31]");
        return 2;
    }
    const bool value = lua_toboolean(state, 2) != 0;
    lua_pushboolean(state, host->runtime->set_story_flag_for_dump(
        static_cast<std::uint8_t>(bit), value));
    return 1;
}

inline int reenter_scene_for_setup(lua_State* state) {
    auto* host = context(state);
    lua_Integer scene = 0;
    if (!integer_argument(state, 1, scene) || scene < 0 || scene > 55) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "scene setup value must be in [0, 55]");
        return 2;
    }
    const bool entered = host->runtime->reenter_field_scene_for_dump(
        static_cast<std::uint16_t>(scene));
    lua_pushboolean(state, entered);
    if (!entered) {
        lua_pushliteral(state,
                        "QA scene setup could not load the requested scene");
        return 2;
    }
    return 1;
}

inline int set_field_mode_flag_for_setup(lua_State* state) {
    auto* host = context(state);
    lua_Integer bit = 0;
    if (!integer_argument(state, 1, bit) || bit < 0 || bit >= 32) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "field mode flag bit must be in [0, 31]");
        return 2;
    }
    const auto mask = std::uint32_t{1U} << static_cast<unsigned int>(bit);
    auto flags = host->runtime->field_mode_flags_for_dump();
    flags = lua_toboolean(state, 2) != 0 ? flags | mask : flags & ~mask;
    host->runtime->seed_field_mode_flags_for_qa(flags);
    lua_pushboolean(state, 1);
    return 1;
}

inline int set_field_mode_effect_flag_for_setup(lua_State* state) {
    auto* host = context(state);
    lua_Integer bit = 0;
    if (!integer_argument(state, 1, bit) || bit < 0 || bit >= 32) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state,
                        "field mode effect flag bit must be in [0, 31]");
        return 2;
    }
    const auto mask = std::uint32_t{1U} << static_cast<unsigned int>(bit);
    auto flags = host->runtime->field_mode_effect_flags_for_dump();
    flags = lua_toboolean(state, 2) != 0 ? flags | mask : flags & ~mask;
    host->runtime->seed_field_mode_effect_flags_for_qa(flags);
    lua_pushboolean(state, 1);
    return 1;
}

inline int set_pending_world_effect_for_setup(lua_State* state) {
    auto* host = context(state);
    const bool pending = lua_toboolean(state, 1) != 0;
    auto& runtime = *host->runtime;
    runtime.seed_conditional_actor_effect_facts_for_dump(
        runtime.world_effect_active_for_dump(), pending,
        runtime.active_effect_index_for_dump(),
        runtime.selected_effect_index_for_dump());
    lua_pushboolean(state, 1);
    return 1;
}

inline int set_field_auxiliary_flag_for_setup(lua_State* state) {
    auto* host = context(state);
    lua_Integer bit = 0;
    if (!integer_argument(state, 1, bit) || bit < 0 || bit >= 32) {
        lua_pushboolean(state, 0);
        lua_pushliteral(state, "field auxiliary flag bit must be in [0, 31]");
        return 2;
    }
    const auto mask = std::uint32_t{1U} << static_cast<unsigned int>(bit);
    auto flags = host->runtime->field_auxiliary_flags_for_dump();
    flags = lua_toboolean(state, 2) != 0 ? flags | mask : flags & ~mask;
    host->runtime->seed_field_auxiliary_flags_for_qa(flags);
    lua_pushboolean(state, 1);
    return 1;
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
    bool waited_for_dialogue_lockout = false;
    for (lua_Integer frame = 0;
         frame < 180 && host->runtime->dialogue_input_locked(); ++frame) {
        waited_for_dialogue_lockout = true;
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
    if (waited_for_dialogue_lockout && !advance_one(*host, 0U)) {
        lua_pushboolean(state, 0);
        lua_pushlstring(state, host->host_error.data(),
                        host->host_error.size());
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
    std::optional<std::uint16_t> stop_scene;
    if (!lua_isnoneornil(state, 3)) {
        lua_Integer scene = 0;
        if (!integer_argument(state, 3, scene) || scene < 0 || scene > 65535) {
            lua_pushboolean(state, 0);
            lua_pushliteral(state, "invalid movement target scene");
            return 2;
        }
        stop_scene = static_cast<std::uint16_t>(scene);
    }
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
                               !runtime.taxi_selector_open_for_dump() &&
                               !runtime.elevator_selector_open_for_dump() &&
                               (!stop_scene.has_value() ||
                                runtime.field_scene_reference_for_dump() !=
                                    *stop_scene);
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
                               runtime.taxi_selector_open_for_dump() ||
                               runtime.elevator_selector_open_for_dump() ||
                               (stop_scene.has_value() &&
                                runtime.field_scene_reference_for_dump() ==
                                    *stop_scene));
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
    lua_pushboolean(state, runtime.elevator_selector_open_for_dump());
    lua_setfield(state, -2, "elevator_open");
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
    lua_pushinteger(state,
                    static_cast<lua_Integer>(dialogue_panel.option_count));
    lua_setfield(state, -2, "panel_option_count");
    lua_pushinteger(state,
                    static_cast<lua_Integer>(dialogue_panel.selected_option));
    lua_setfield(state, -2, "panel_selected_option");
    lua_createtable(state,
                    static_cast<int>(dialogue_panel.option_count), 0);
    for (std::size_t index = 0U;
         index < dialogue_panel.option_count &&
         index < dialogue_panel.options.size(); ++index) {
        const auto label = dialogue_panel.options[index].source_label;
        lua_pushlstring(state, label.data(), label.size());
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    lua_setfield(state, -2, "panel_options");
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

    lua_createtable(state, 0,
                    static_cast<int>(kWalkthroughGroupItemIds.size()));
    for (std::size_t index = 0U;
         index < kWalkthroughGroupItemIds.size(); ++index) {
        const auto flags = runtime.group_item_flags_for_dump();
        lua_pushboolean(state, flags.has_value() &&
            ((*flags & (std::uint32_t{1U} << index)) != 0U));
        const auto item_id = kWalkthroughGroupItemIds[index];
        lua_setfield(state, -2, std::string(item_id).c_str());
    }
    lua_setfield(state, -2, "group_items");

    lua_createtable(state, 0,
                    static_cast<int>(kWalkthroughStoryFactIds.size()));
    const auto story_flags = runtime.field_world_fact_flags_for_dump();
    for (std::size_t index = 0U;
         index < kWalkthroughStoryFactIds.size(); ++index) {
        lua_pushboolean(state,
            (story_flags & (std::uint32_t{1U} << index)) != 0U);
        const auto fact_id = kWalkthroughStoryFactIds[index];
        lua_setfield(state, -2, std::string(fact_id).c_str());
    }
    lua_setfield(state, -2, "story_facts");

    lua_createtable(state, 0,
                    static_cast<int>(kWalkthroughStoryFactIds.size()));
    for (std::size_t index = 0U;
         index < kWalkthroughStoryFactIds.size(); ++index) {
        lua_pushinteger(state, static_cast<lua_Integer>(index));
        const auto fact_id = kWalkthroughStoryFactIds[index];
        lua_setfield(state, -2, std::string(fact_id).c_str());
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
                            (key.contains("item") || key.contains("inventor") ||
                                key.contains("equip")) &&
                                key != "group_item_flags");
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

inline std::optional<QString> first_json_difference(
    const QJsonValue& expected, const QJsonValue& actual,
    const QString& path = QStringLiteral("$")) {
    if (expected.type() != actual.type()) {
        return path + QStringLiteral(": JSON type changed");
    }
    if (expected.isObject()) {
        const auto expected_object = expected.toObject();
        const auto actual_object = actual.toObject();
        for (const auto& key : expected_object.keys()) {
            if (!actual_object.contains(key)) {
                return path + QStringLiteral(".") + key +
                    QStringLiteral(": missing after restore");
            }
            const auto difference = first_json_difference(
                expected_object.value(key), actual_object.value(key),
                path + QStringLiteral(".") + key);
            if (difference.has_value()) return difference;
        }
        for (const auto& key : actual_object.keys()) {
            if (!expected_object.contains(key)) {
                return path + QStringLiteral(".") + key +
                    QStringLiteral(": added after restore");
            }
        }
        return std::nullopt;
    }
    if (expected.isArray()) {
        const auto expected_array = expected.toArray();
        const auto actual_array = actual.toArray();
        if (expected_array.size() != actual_array.size()) {
            return path + QStringLiteral(": array size changed");
        }
        for (qsizetype index = 0; index < expected_array.size(); ++index) {
            const auto difference = first_json_difference(
                expected_array.at(index), actual_array.at(index),
                path + QStringLiteral("[") + QString::number(index) +
                    QStringLiteral("]"));
            if (difference.has_value()) return difference;
        }
        return std::nullopt;
    }
    if (expected != actual) {
        return path + QStringLiteral(": value changed");
    }
    return std::nullopt;
}

inline std::optional<std::string> validate_seg1_save_snapshot(
    const QJsonObject& snapshot) {
    const auto require_keys = [](const QJsonObject& object,
                                 const std::initializer_list<const char*> keys,
                                 const std::string_view object_name)
        -> std::optional<std::string> {
        for (const auto* key : keys) {
            if (!object.contains(QLatin1String(key))) {
                return std::string(object_name) + "." + key + " is missing";
            }
        }
        return std::nullopt;
    };
    if (const auto missing = require_keys(
            snapshot, {"kind", "scene", "party", "spell_tables",
                       "funds_available", "funds", "story", "script_facts",
                       "run_registry"}, "save")) {
        return missing;
    }
    const auto scene = snapshot.value(QStringLiteral("scene")).toObject();
    if (const auto missing = require_keys(
            scene, {"reference", "mode", "player_x", "player_y", "heading",
                    "direction", "camera_x", "camera_y", "target_heading"},
            "save.scene")) {
        return missing;
    }
    const auto party = snapshot.value(QStringLiteral("party")).toObject();
    if (const auto missing = require_keys(
            party, {"active_slot", "members", "primary_compact_marker",
                    "secondary_compact_marker"}, "save.party")) {
        return missing;
    }
    const auto party_members =
        party.value(QStringLiteral("members")).toArray();
    if (party_members.size() != static_cast<qsizetype>(
            FieldPartyRosterRuntime::kSlotCount)) {
        return "save.party.members has the wrong slot count";
    }
    for (qsizetype index = 0; index < party_members.size(); ++index) {
        const auto member = party_members.at(index).toObject();
        const auto member_name =
            "save.party.members[" + std::to_string(index) + "]";
        if (const auto missing = require_keys(
                member, {"active", "name", "portrait_family", "record"},
                member_name)) {
            return missing;
        }
        const auto record = member.value(QStringLiteral("record")).toObject();
        if (const auto missing = require_keys(
                record, {"inventory_item_codes", "inventory_phase_bytes",
                         "karma", "attributes", "skills"},
                member_name + ".record")) {
            return missing;
        }
        if (!record.value(QStringLiteral("inventory_item_codes")).isArray() ||
            !record.value(QStringLiteral("inventory_phase_bytes")).isArray()) {
            return member_name + ".record inventory fields are not arrays";
        }
    }

    const auto story = snapshot.value(QStringLiteral("story")).toObject();
    if (const auto missing = require_keys(
            story,
            {"exit_world_flags", "mode_flags_resolved", "auxiliary_flags",
             "render_effect_flags", "dialogue_register", "dialogue_flags",
             "map_event_flags", "group_item_flags", "cyberdeck_available",
             "matrix_passcode_flags", "matrix_field_state", "karma_counter",
             "passive_ic_counter", "cyberdeck_record", "runner_feature_flags",
             "heat_level", "strike_team_counter",
             "runner_contact_progress_counters", "runner_information_flags",
             "walk_fast", "notebook_current_run_available",
             "notebook_tips_resolved", "notebook_tip_flags",
             "notebook_contacts_flags", "notebook_referral_flags",
             "notebook_shadowrunner_flags", "world_frame_counter",
             "world_encounter_sequence", "source_random_seed"}, "save.story")) {
        return missing;
    }
    if (story.value(QStringLiteral("notebook_tip_flags")).toArray().size() != 3) {
        return "save.story.notebook_tip_flags has the wrong word count";
    }
    const auto run_registry =
        snapshot.value(QStringLiteral("run_registry")).toObject();
    if (!run_registry.contains(QStringLiteral("schema_version")) ||
        !snapshot.value(QStringLiteral("script_facts")).isObject()) {
        return "save.run_registry schema or save.script_facts is missing";
    }
    return std::nullopt;
}

inline std::optional<std::string> read_walkthrough_fixture(
    const std::filesystem::path& path, const std::string_view checkpoint,
    QJsonObject& snapshot) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return "could not open walkthrough fixture: " + path.string();
    }
    const std::string bytes((std::istreambuf_iterator<char>(input)),
                            std::istreambuf_iterator<char>());
    QJsonParseError parse_error;
    const auto document = QJsonDocument::fromJson(
        QByteArray(bytes.data(), static_cast<qsizetype>(bytes.size())),
        &parse_error);
    if (parse_error.error != QJsonParseError::NoError ||
        !document.isObject()) {
        return "invalid walkthrough fixture JSON at byte " +
            std::to_string(parse_error.offset) + ": " +
            parse_error.errorString().toStdString();
    }
    const auto fixture = document.object();
    if (fixture.value(QStringLiteral("format")).toString() !=
            QString::fromLatin1(kSeg1EndFixtureFormat.data(),
                                static_cast<qsizetype>(
                                    kSeg1EndFixtureFormat.size())) ||
        fixture.value(QStringLiteral("checkpoint")).toString() !=
            QString::fromLatin1(checkpoint.data(),
                                static_cast<qsizetype>(checkpoint.size())) ||
        fixture.value(QStringLiteral("generated_from_commit")).toString().isEmpty()) {
        return "walkthrough fixture header is missing its format, checkpoint, "
               "or source commit";
    }
    const auto snapshot_value = fixture.value(QStringLiteral("snapshot"));
    if (!snapshot_value.isObject()) {
        return "walkthrough fixture snapshot is not an object";
    }
    snapshot = snapshot_value.toObject();
    return validate_seg1_save_snapshot(snapshot);
}

inline QByteArray walkthrough_fixture_bytes(
    const QJsonObject& snapshot, const std::string_view checkpoint) {
    QJsonObject fixture;
    fixture.insert(QStringLiteral("format"),
                   QString::fromLatin1(kSeg1EndFixtureFormat.data(),
                                       static_cast<qsizetype>(
                                           kSeg1EndFixtureFormat.size())));
    fixture.insert(QStringLiteral("checkpoint"),
                   QString::fromLatin1(checkpoint.data(),
                                       static_cast<qsizetype>(
                                           checkpoint.size())));
    fixture.insert(QStringLiteral("generated_from_commit"),
                   QString::fromLatin1(kSeg1EndFixtureSourceCommit.data(),
                                       static_cast<qsizetype>(
                                           kSeg1EndFixtureSourceCommit.size())));
    fixture.insert(QStringLiteral("snapshot"), snapshot);
    return QJsonDocument(fixture).toJson(QJsonDocument::Indented);
}

inline std::optional<std::string> write_walkthrough_fixture(
    const QJsonObject& snapshot, const std::string_view checkpoint,
    const std::filesystem::path& output_path,
    const std::optional<std::filesystem::path>& compare_path) {
    std::error_code filesystem_error;
    if (!output_path.parent_path().empty()) {
        std::filesystem::create_directories(output_path.parent_path(),
                                            filesystem_error);
    }
    if (filesystem_error) {
        return "could not create walkthrough fixture directory: " +
            filesystem_error.message();
    }
    const auto bytes = walkthrough_fixture_bytes(snapshot, checkpoint);
    std::ofstream output(output_path, std::ios::binary | std::ios::trunc);
    if (!output) {
        return "could not write walkthrough fixture: " + output_path.string();
    }
    output.write(bytes.constData(), static_cast<std::streamsize>(bytes.size()));
    output.close();
    if (!output) {
        return "failed while writing walkthrough fixture: " +
            output_path.string();
    }
    if (!compare_path.has_value()) return std::nullopt;

    std::ifstream reference(*compare_path, std::ios::binary);
    if (!reference) {
        return "could not open reference walkthrough fixture: " +
            compare_path->string();
    }
    const std::string reference_bytes(
        (std::istreambuf_iterator<char>(reference)),
        std::istreambuf_iterator<char>());
    if (reference_bytes.size() != static_cast<std::size_t>(bytes.size()) ||
        !std::equal(reference_bytes.begin(), reference_bytes.end(),
                    bytes.constData())) {
        std::size_t first_difference = 0U;
        const auto common_size = std::min(
            reference_bytes.size(), static_cast<std::size_t>(bytes.size()));
        while (first_difference < common_size &&
               reference_bytes[first_difference] == bytes.constData()[first_difference]) {
            ++first_difference;
        }
        return "regenerated walkthrough fixture differs from " +
            compare_path->string() + " at byte " +
            std::to_string(first_difference) + " (generated " +
            std::to_string(bytes.size()) + " bytes, reference " +
            std::to_string(reference_bytes.size()) + " bytes)";
    }
    return std::nullopt;
}

inline std::optional<std::string> restore_fixture_and_compare(
    FieldSceneRuntime& runtime, const QJsonObject& expected_snapshot) {
    QString error;
    if (!runtime.restore_game_save_snapshot_for_dump(expected_snapshot, &error)) {
        return "fresh runtime could not restore walkthrough fixture: " +
            error.toStdString();
    }
    const auto restored_snapshot =
        runtime.capture_game_save_snapshot_for_dump(&error);
    if (!restored_snapshot.has_value()) {
        return "fresh runtime could not capture restored walkthrough fixture: " +
            error.toStdString();
    }
    const auto difference = first_json_difference(
        QJsonValue(expected_snapshot), QJsonValue(*restored_snapshot));
    if (difference.has_value()) {
        return "restored fixture mismatch at " + difference->toStdString();
    }
    return std::nullopt;
}

inline std::optional<std::string_view> input_fixture_checkpoint(
    const std::string_view segment) {
    if (segment == "seg1") return "seg1_end";
    if (segment == "w3") return "seg1_end";
    if (segment == "w3_b2") return "seg1_end";
    if (segment == "w4") return "seg2_b2_end";
    if (segment == "w4_b6") return "seg3_b5_end";
    if (segment == "w4_b7") return "seg3_b6_end";
    if (segment == "w5_b13") return "seg2_b2_end";
    if (segment == "w5_b13_icarus_entry") return "seg4_downtown_arrival_end";
    if (segment == "w5_b23") return "seg4_downtown_arrival_end";
    if (segment == "w5_b24") return "seg4_b23_end";
    if (segment == "w5_b25_gate") return "seg4_b23_end";
    if (segment == "w5_b25") return "seg4_b24_end";
    if (segment == "w5_b29") return "seg4_b25_end";
    if (segment == "w5_b14_entry") return "seg4_b29_end";
    if (segment == "w6_b17") return "seg4_b29_end";
    if (segment == "w6_b18") return "seg4_b29_end";
    if (segment == "w6_b19") return "seg4_b29_end";
    if (segment == "w6_b20") return "seg4_b29_end";
    if (segment == "w5") return "seg3_end";
    if (segment == "w6") return "seg4_end";
    if (segment == "w7") return "seg5_end";
    if (segment == "w8") return "seg6_end";
    return std::nullopt;
}

inline std::optional<std::string_view> output_fixture_checkpoint(
    const std::string_view segment) {
    if (segment == "seg1") return "seg1_end";
    if (segment == "w3") return "seg2_end";
    if (segment == "w3_b2") return "seg2_b2_end";
    if (segment == "w4") return "seg3_b5_end";
    if (segment == "w4_b6") return "seg3_b6_end";
    if (segment == "w4_b7") return "seg3_b7_end";
    if (segment == "w5_b13") return "seg4_downtown_arrival_end";
    if (segment == "w5_b13_icarus_entry") return "seg4_icarus_entry_end";
    if (segment == "w5_b23") return "seg4_b23_end";
    if (segment == "w5_b24") return "seg4_b24_salish_end";
    if (segment == "w5_b25_gate") return "seg4_b25_gate_end";
    if (segment == "w5_b25") return "seg4_b25_end";
    if (segment == "w5_b29") return "seg4_b29_end";
    if (segment == "w5_b14_entry") return "seg4_b14_entry_end";
    if (segment == "w6_b17") return "seg5_b17_end";
    if (segment == "w6_b18") return "seg5_b18_end";
    if (segment == "w6_b19") return "seg5_b19_end";
    if (segment == "w6_b20") return "seg5_b20_end";
    if (segment == "w5") return "seg4_end";
    if (segment == "w6") return "seg5_end";
    if (segment == "w7") return "seg6_end";
    if (segment == "w8") return "seg7_end";
    return std::nullopt;
}

inline std::optional<std::string> verify_seg1_save_round_trip(
    FieldSceneRuntime& source_runtime,
    const shadowrun::ghidra_conversion::integration::StartupIntegrationState&
        startup_state) {
    QString error;
    const auto source_snapshot =
        source_runtime.capture_game_save_snapshot_for_dump(&error);
    if (!source_snapshot.has_value()) {
        return "could not capture segment-1 save snapshot: " +
            error.toStdString();
    }

    if (const auto validation_error =
            validate_seg1_save_snapshot(*source_snapshot)) {
        return validation_error;
    }

    std::vector<std::string> restored_source_events;
    std::vector<std::uint16_t> restored_audio_events;
    FieldSceneRuntime restored_runtime(
        [&](const std::string_view event) {
            restored_source_events.emplace_back(event);
        },
        [&](const std::uint16_t event) {
            restored_audio_events.push_back(event);
        },
        [](const std::uint16_t) {});
    restored_runtime.set_source_random_seed_for_dump(kSourceRandomSeed);
    if (!restored_runtime.bootstrap(startup_state)) {
        return "fresh runtime bootstrap failed before restore: " +
            restored_runtime.failure();
    }
    const auto restore_error = restore_fixture_and_compare(
        restored_runtime, *source_snapshot);
    if (restore_error.has_value()) {
        return "fresh runtime save round-trip failed: " + *restore_error;
    }
    return std::nullopt;
}

} // namespace walkthrough_detail

inline int run_walkthrough_qa(
    shadowrun::ghidra_conversion::integration::StartupIntegrationState
        startup_state,
    const std::filesystem::path& asset_root,
    const std::string& segment,
    const std::filesystem::path& output_directory,
    const bool reach_mode,
    const std::optional<std::filesystem::path>& from_fixture_path =
        std::nullopt,
    const std::optional<std::filesystem::path>& write_fixture_path =
        std::nullopt,
    const std::optional<std::filesystem::path>& compare_fixture_path =
        std::nullopt) {
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

    bool resumed_from_fixture = false;
    if (from_fixture_path.has_value()) {
        QJsonObject fixture_snapshot;
        const auto checkpoint = segment == "fixture_restore"
            ? std::optional<std::string_view>{}
            : input_fixture_checkpoint(segment);
        const auto checkpoint_from_path =
            from_fixture_path->stem().string();
        const std::string_view expected_checkpoint = checkpoint.has_value()
            ? *checkpoint
            : std::string_view{checkpoint_from_path};
        const auto fixture_error = read_walkthrough_fixture(
            *from_fixture_path, expected_checkpoint, fixture_snapshot);
        if (fixture_error.has_value()) {
            std::cerr << "walkthrough fixture load failed: " << *fixture_error
                      << '\n';
            return 1;
        }
        const auto restore_error = restore_fixture_and_compare(
            runtime, fixture_snapshot);
        if (restore_error.has_value()) {
            std::cerr << "walkthrough fixture restore failed: "
                      << *restore_error << '\n';
            return 1;
        }
        std::cout << "walkthrough from-fixture: " << expected_checkpoint
                  << " restored exactly\n";
        if (segment == "fixture_restore") {
            if (runtime.opening_auto_walk_active_for_dump()) {
                std::cerr << "walkthrough fixture restore retained the "
                             "New Game-only opening walk\n";
                return 1;
            }
            return 0;
        }
        if (segment == "seg1" || segment == "w3" ||
            segment == "w3_b2" || segment == "w4" ||
            segment == "w4_b6" ||
            segment == "w4_b7" ||
            segment == "w5_b13" ||
            segment == "w5_b23" ||
            segment == "w5_b24" ||
            segment == "w5_b25_gate" ||
            segment == "w5_b25" ||
            segment == "w5_b29" ||
            segment == "w5_b14_entry" ||
            segment == "w6_b17" ||
            segment == "w6_b18" ||
            segment == "w6_b19" ||
            segment == "w6_b20" ||
            segment == "w5" || segment == "w6" || segment == "w7" ||
            segment == "w8") {
            resumed_from_fixture = true;
        }
    } else if (segment == "fixture_restore") {
        std::cerr << "walkthrough: fixture_restore requires --from-fixture\n";
        return 3;
    }

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
    host.from_fixture = resumed_from_fixture;
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
    add_host_function(state, host, "grant_nuyen", grant_nuyen);
    add_host_function(state, host, "grant_karma", grant_karma);
    add_host_function(state, host, "grant_group_item", grant_group_item);
    add_host_function(state, host, "set_story_fact_for_setup",
                      set_story_fact_for_setup);
    add_host_function(state, host, "reenter_scene_for_setup",
                      reenter_scene_for_setup);
    add_host_function(state, host, "set_field_mode_flag_for_setup",
                      set_field_mode_flag_for_setup);
    add_host_function(state, host, "set_field_mode_effect_flag_for_setup",
                      set_field_mode_effect_flag_for_setup);
    add_host_function(state, host, "set_pending_world_effect_for_setup",
                      set_pending_world_effect_for_setup);
    add_host_function(state, host, "set_field_auxiliary_flag_for_setup",
                      set_field_auxiliary_flag_for_setup);
    add_host_function(state, host, "snapshot", snapshot);
    const auto output_string = output_directory.string();
    lua_pushlstring(state, output_string.data(), output_string.size());
    lua_setfield(state, -2, "output_directory");
    const auto asset_string = asset_root.string();
    lua_pushlstring(state, asset_string.data(), asset_string.size());
    lua_setfield(state, -2, "asset_root");
    lua_pushboolean(state, host.from_fixture);
    lua_setfield(state, -2, "from_fixture");
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
    bool save_round_trip_passed = true;
    if (segment == "seg1" && !reach_mode && passed) {
        const auto round_trip_error = verify_seg1_save_round_trip(
            runtime, startup_state);
        if (round_trip_error.has_value()) {
            save_round_trip_passed = false;
            std::cerr << "walkthrough save round-trip failed: "
                      << *round_trip_error << '\n';
        } else {
            std::cout << "walkthrough save round-trip: seg1_end passed\n";
        }
    }
    bool fixture_generation_passed = true;
    const auto output_checkpoint = output_fixture_checkpoint(segment);
    if ((write_fixture_path.has_value() || compare_fixture_path.has_value()) &&
        output_checkpoint.has_value() && !reach_mode && passed &&
        save_round_trip_passed) {
        if (!write_fixture_path.has_value()) {
            fixture_generation_passed = false;
            std::cerr << "walkthrough fixture generation failed: "
                         "--compare-fixture requires --write-fixture\n";
        } else {
            QString snapshot_error;
            const auto snapshot = runtime.capture_game_save_snapshot_for_dump(
                &snapshot_error);
            if (!snapshot.has_value()) {
                fixture_generation_passed = false;
                std::cerr << "walkthrough fixture generation failed: "
                          << snapshot_error.toStdString() << '\n';
            } else {
                const auto write_error = write_walkthrough_fixture(
                    *snapshot, *output_checkpoint, *write_fixture_path,
                    compare_fixture_path);
                if (write_error.has_value()) {
                    fixture_generation_passed = false;
                    std::cerr << "walkthrough fixture generation failed: "
                              << *write_error << '\n';
                } else {
                    std::cout << "walkthrough fixture: " << *output_checkpoint
                              << " generated"
                              << (compare_fixture_path.has_value()
                                      ? " and byte-exact comparison passed\n"
                                      : "\n");
                }
            }
        }
    } else if (write_fixture_path.has_value() ||
               compare_fixture_path.has_value()) {
        fixture_generation_passed = false;
        std::cerr << "walkthrough fixture generation requires a passing "
                     "seg1 gate run\n";
    }
    write_lua_report(state, output_directory, segment, reach_mode);
    lua_close(state);
    return (passed && save_round_trip_passed && fixture_generation_passed) ||
                   reach_mode
        ? 0
        : 1;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime::qa
