#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#include <algorithm>
#include <cstdint>
#include <set>
#include <system_error>
#include <type_traits>
#include <utility>

namespace shadowrun::ghidra_conversion::scripting {
namespace {

constexpr std::size_t kMaximumScriptModules = 128U;
constexpr std::size_t kMaximumMapEventRules = 512U;
// The command overview has seven source rows; Pocket Secretary's extended
// caller can expose six. Keep the generic script-menu bound at the larger
// source-defined menu size.
constexpr std::size_t kMaximumMenuChoices = 7U;
// Static script catalogs may describe source tables such as all 13 Magic
// spells or 32 shop headings; live dialogue menus keep the seven-row cap.
constexpr std::size_t kMaximumMenuCatalogChoices = 32U;
constexpr std::size_t kMaximumStringMapEntries = 128U;
constexpr std::size_t kMaximumShopGroups = 8U;
constexpr std::size_t kMaximumShopOffers = 8U;
constexpr std::size_t kMaximumItemCatalogEntries = 50U;
constexpr std::size_t kMaximumItemDetailTextEntries = 32U;
constexpr std::size_t kMaximumIdentifierLength = 96U;
constexpr std::size_t kMaximumStoryFacts = 4096U;
constexpr std::size_t kMaximumDialogueTextEntries = 1024U;
constexpr std::size_t kMaximumDialogueTextBytes = 8192U;
constexpr int kScriptInstructionBudget = 100000;

std::string stack_error(lua_State* state) {
    const auto* message = lua_tostring(state, -1);
    return message == nullptr ? "Lua reported a non-string error"
                              : std::string(message);
}

void instruction_budget_hook(lua_State* state, lua_Debug*) {
    luaL_error(state, "gameplay script exceeded its instruction budget");
}

class ScopedInstructionBudget final {
public:
    explicit ScopedInstructionBudget(lua_State* state) : state_(state) {
        lua_sethook(state_, instruction_budget_hook, LUA_MASKCOUNT,
                    kScriptInstructionBudget);
    }
    ~ScopedInstructionBudget() { lua_sethook(state_, nullptr, 0, 0); }

    ScopedInstructionBudget(const ScopedInstructionBudget&) = delete;
    ScopedInstructionBudget& operator=(const ScopedInstructionBudget&) = delete;

private:
    lua_State* state_;
};

bool read_string(lua_State* state, const int index, std::string& value) {
    if (lua_type(state, index) != LUA_TSTRING) {
        return false;
    }
    std::size_t length = 0U;
    const auto* text = lua_tolstring(state, index, &length);
    if (text == nullptr || length == 0U ||
        length > kMaximumIdentifierLength) {
        return false;
    }
    value.assign(text, length);
    return true;
}

bool read_integer(lua_State* state, const int index,
                  const std::uint32_t maximum, std::uint32_t& value) {
    if (!lua_isinteger(state, index)) {
        return false;
    }
    const auto number = lua_tointeger(state, index);
    if (number < 0 ||
        static_cast<lua_Unsigned>(number) > maximum) {
        return false;
    }
    value = static_cast<std::uint32_t>(number);
    return true;
}

bool read_field_string(lua_State* state, const int table_index,
                       const char* field, std::string& value) {
    lua_getfield(state, table_index, field);
    const bool valid = read_string(state, -1, value);
    lua_pop(state, 1);
    return valid;
}

bool read_field_integer(lua_State* state, const int table_index,
                        const char* field, const std::uint32_t maximum,
                        std::uint32_t& value) {
    lua_getfield(state, table_index, field);
    const bool valid = read_integer(state, -1, maximum, value);
    lua_pop(state, 1);
    return valid;
}

bool valid_identifier(const std::string_view value) {
    return !value.empty() && value.size() <= kMaximumIdentifierLength &&
           std::all_of(value.begin(), value.end(), [](const char character) {
               return (character >= 'a' && character <= 'z') ||
                      (character >= 'A' && character <= 'Z') ||
                      (character >= '0' && character <= '9') ||
                      character == '_' || character == '-' ||
                      character == '.';
           });
}

void open_safe_libraries(lua_State* state) {
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

    // Scripts are event handlers and menu descriptions, not plug-ins with
    // filesystem, process, or dynamic module access.
    for (const char* name : {"dofile", "loadfile", "load",
                             "collectgarbage", "getmetatable",
                             "setmetatable"}) {
        lua_pushnil(state);
        lua_setglobal(state, name);
    }
}

bool field_flag(const std::string_view name, std::uint32_t& flag) {
    if (name == "new_game_opening_completed") {
        flag = field_map_event_flag(
            FieldMapEventStateFlag::new_game_opening_completed);
        return true;
    }
    return false;
}

bool field_trigger(const std::string_view name,
                   FieldMapEventTrigger& trigger) {
    if (name == "scene_entry") {
        trigger = FieldMapEventTrigger::scene_entry;
        return true;
    }
    if (name == "actor_within_scope") {
        trigger = FieldMapEventTrigger::actor_within_scope;
        return true;
    }
    return false;
}

bool field_repeat_policy(const std::string_view name,
                         FieldMapEventRepeatPolicy& policy) {
    if (name == "every_match") {
        policy = FieldMapEventRepeatPolicy::every_match;
        return true;
    }
    if (name == "once_per_scene_visit") {
        policy = FieldMapEventRepeatPolicy::once_per_scene_visit;
        return true;
    }
    if (name == "once_per_session") {
        policy = FieldMapEventRepeatPolicy::once_per_session;
        return true;
    }
    return false;
}

bool field_action(const std::string_view name,
                  FieldMapEventScriptAction& action) {
    if (name == "run_lua_handler") {
        action = FieldMapEventScriptAction::run_lua_handler;
        return true;
    }
    if (name == "dispatch_native_script") {
        action = FieldMapEventScriptAction::dispatch_native_script;
        return true;
    }
    return false;
}

bool parse_coordinate_scope(lua_State* state, const int rule_index,
                            std::optional<FieldMapEventCoordinateScope>& scope) {
    lua_getfield(state, rule_index, "scope");
    if (lua_isnil(state, -1)) {
        lua_pop(state, 1);
        scope.reset();
        return true;
    }
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto scope_index = lua_absindex(state, -1);
    FieldMapEventCoordinateScope parsed{};
    std::uint32_t min_x = 0U;
    std::uint32_t max_x = 0U;
    std::uint32_t min_y = 0U;
    std::uint32_t max_y = 0U;
    const bool valid =
        read_field_integer(state, scope_index, "min_x", 0xFFFFU,
                           min_x) &&
        read_field_integer(state, scope_index, "max_x", 0xFFFFU,
                           max_x) &&
        read_field_integer(state, scope_index, "min_y", 0xFFFFU,
                           min_y) &&
        read_field_integer(state, scope_index, "max_y", 0xFFFFU,
                           max_y) &&
        min_x <= max_x && min_y <= max_y;
    lua_pop(state, 1);
    if (valid) {
        parsed.min_x = static_cast<std::uint16_t>(min_x);
        parsed.max_x = static_cast<std::uint16_t>(max_x);
        parsed.min_y = static_cast<std::uint16_t>(min_y);
        parsed.max_y = static_cast<std::uint16_t>(max_y);
        scope = parsed;
    }
    return valid;
}

bool parse_required_flags(lua_State* state, const int rule_index,
                          std::uint32_t& flags) {
    lua_getfield(state, rule_index, "requires");
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        return false;
    }
    const auto flags_index = lua_absindex(state, -1);
    const auto count = lua_rawlen(state, flags_index);
    bool valid = count <= 32U;
    for (std::size_t index = 1U; valid && index <= count; ++index) {
        lua_rawgeti(state, flags_index, static_cast<lua_Integer>(index));
        std::string name;
        std::uint32_t flag = 0U;
        valid = read_string(state, -1, name) && field_flag(name, flag);
        if (valid) {
            flags |= flag;
        }
        lua_pop(state, 1);
    }
    lua_pop(state, 1);
    return valid;
}

bool parse_event_rule(lua_State* state, const int index,
                      FieldMapEventRule& rule) {
    if (!lua_istable(state, index)) {
        return false;
    }
    std::uint32_t scene_id = 0U;
    std::string trigger_name;
    std::string repeat_name;
    std::string action_name;
    std::string handler_name;
    if (!read_field_integer(state, index, "id", 0xFFFFFFFFU, rule.id) ||
        rule.id == 0U ||
        !read_field_integer(state, index, "scene", 0xFFFFU, scene_id) ||
        !read_field_string(state, index, "trigger", trigger_name) ||
        !field_trigger(trigger_name, rule.trigger) ||
        !read_field_string(state, index, "repeat", repeat_name) ||
        !field_repeat_policy(repeat_name, rule.repeat_policy) ||
        !read_field_string(state, index, "action", action_name) ||
        !field_action(action_name, rule.action) ||
        !parse_coordinate_scope(state, index, rule.scope) ||
        !parse_required_flags(state, index, rule.required_state_flags)) {
        return false;
    }
    if (rule.action == FieldMapEventScriptAction::run_lua_handler &&
        (!read_field_string(state, index, "handler", handler_name) ||
         !valid_identifier(handler_name))) {
        return false;
    }
    rule.scene_id = static_cast<std::uint16_t>(scene_id);
    rule.handler = std::move(handler_name);
    return true;
}

bool parse_menu_choices(lua_State* state, const int index,
                        const std::string_view menu_id,
                        const std::string_view text_profile,
                        const std::string_view speaker_id,
                        ScriptMenuDefinition& menu,
                        const std::size_t maximum_choices =
                            kMaximumMenuChoices) {
    if (!lua_istable(state, index) || !valid_identifier(menu_id) ||
        !valid_identifier(text_profile)) {
        return false;
    }
    const auto count = lua_rawlen(state, index);
    if (count > maximum_choices) {
        return false;
    }
    menu.id = std::string(menu_id);
    menu.text_profile = std::string(text_profile);
    menu.speaker_id = std::string(speaker_id);
    menu.choices.reserve(count);
    std::set<std::string> choice_ids;
    for (std::size_t choice_index = 1U; choice_index <= count;
         ++choice_index) {
        lua_rawgeti(state, index, static_cast<lua_Integer>(choice_index));
        ScriptMenuChoice choice;
        const bool valid = read_string(state, -1, choice.id) &&
                           valid_identifier(choice.id);
        lua_pop(state, 1);
        if (!valid || !choice_ids.insert(choice.id).second) {
            return false;
        }
        menu.choices.push_back(std::move(choice));
    }
    return true;
}

bool parse_explicit_menu_text_keys(lua_State* state,
                                   const int title_key_index,
                                   const int label_keys_index,
                                   ScriptMenuDefinition& menu) {
    if (!lua_isnil(state, title_key_index)) {
        if (!read_string(state, title_key_index, menu.title_key) ||
            !valid_identifier(menu.title_key)) {
            return false;
        }
    }
    if (!lua_istable(state, label_keys_index) ||
        lua_rawlen(state, label_keys_index) != menu.choices.size()) {
        return false;
    }
    for (std::size_t index = 0; index < menu.choices.size(); ++index) {
        auto& label_key = menu.choices[index].label_key;
        lua_rawgeti(state, label_keys_index,
                    static_cast<lua_Integer>(index + 1U));
        const bool valid = read_string(state, -1, label_key) &&
                           valid_identifier(label_key);
        lua_pop(state, 1);
        if (!valid) {
            return false;
        }
    }
    return true;
}

} // namespace

struct GameScriptRuntime::Impl {
    lua_State* state = nullptr;
    lua_State* task_state = nullptr;
    int modules_reference = LUA_NOREF;
    int task_reference = LUA_NOREF;
    bool task_waiting = false;
    std::optional<ScriptNativeOperation> pending_task_operation;
    std::optional<ScriptNativeOperation> active_native_operation;
    StoryFacts story_facts;
    StoryFactReader story_fact_reader;
    StoryFactWriter story_fact_writer;
    std::unordered_map<std::string, std::string> dialogue_texts;
    std::string location_type;
    std::string location_id;
    std::string location_text_profile;
    std::string location_speaker_id;
    ScriptFieldModeContext field_mode_context{};
    const ScriptActorBehaviorContext* active_actor_context = nullptr;
    std::string bridge_failure;
    std::string failure;

    static Impl* from_upvalue(lua_State* lua) {
        return static_cast<Impl*>(lua_touserdata(lua, lua_upvalueindex(1)));
    }

    static int state_index(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || lua_type(lua, 2) != LUA_TSTRING) {
            lua_pushnil(lua);
            return 1;
        }
        std::string name;
        if (!read_string(lua, 2, name) || !valid_identifier(name)) {
            lua_pushnil(lua);
            return 1;
        }
        const auto fact = self->story_fact_reader
                              ? self->story_fact_reader(name)
                              : [&]() -> std::optional<bool> {
                                    const auto stored =
                                        self->story_facts.find(name);
                                    if (stored == self->story_facts.end()) {
                                        return std::nullopt;
                                    }
                                    return stored->second;
                                }();
        if (!fact.has_value()) {
            lua_pushnil(lua);
        } else {
            lua_pushboolean(lua, *fact ? 1 : 0);
        }
        return 1;
    }

    static int state_newindex(lua_State* lua) {
        auto* self = from_upvalue(lua);
        const char* error = nullptr;
        if (self == nullptr) {
            error = "game.state has no native story-fact store";
        } else {
            {
                std::string name;
                if (lua_type(lua, 2) != LUA_TSTRING ||
                    !read_string(lua, 2, name) ||
                    !valid_identifier(name) ||
                    lua_type(lua, 3) != LUA_TBOOLEAN) {
                    error =
                        "game.state accepts named boolean story facts only";
                } else if (self->story_fact_writer) {
                    const bool value = lua_toboolean(lua, 3) != 0;
                    if (self->story_fact_writer(name, value)) {
                        return 0;
                    }
                    error = "game.state property is not writable";
                } else if (
                    self->story_facts.find(name) ==
                        self->story_facts.end() &&
                    self->story_facts.size() >= kMaximumStoryFacts) {
                    error = "game.state has reached its fact limit";
                } else {
                    self->story_facts[std::move(name)] =
                        lua_toboolean(lua, 3) != 0;
                    return 0;
                }
            }
        }
        return luaL_error(lua, "%s", error);
    }

    static int location_index(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || lua_type(lua, 2) != LUA_TSTRING) {
            lua_pushnil(lua);
            return 1;
        }
        std::string field;
        if (!read_string(lua, 2, field)) {
            lua_pushnil(lua);
            return 1;
        }
        std::string_view value;
        if (field == "type") {
            value = self->location_type;
        } else if (field == "id") {
            value = self->location_id;
        } else if (field == "text_profile") {
            value = self->location_text_profile;
        } else if (field == "speaker_id") {
            value = self->location_speaker_id;
        } else {
            lua_pushnil(lua);
            return 1;
        }
        if (value.empty()) {
            lua_pushnil(lua);
        } else {
            lua_pushlstring(lua, value.data(), value.size());
        }
        return 1;
    }

    static int location_newindex(lua_State* lua) {
        return luaL_error(lua, "game.location is native read-only context");
    }

    static int field_context_index(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || lua_type(lua, 2) != LUA_TSTRING) {
            lua_pushnil(lua);
            return 1;
        }
        std::string field;
        if (!read_string(lua, 2, field)) {
            lua_pushnil(lua);
            return 1;
        }
        if (field == "mode") {
            lua_pushinteger(lua, self->field_mode_context.mode);
        } else if (field == "event_code") {
            lua_pushinteger(lua, self->field_mode_context.event_code);
        } else if (field == "alternate_result") {
            lua_pushinteger(lua,
                            self->field_mode_context.alternate_result);
        } else {
            lua_pushnil(lua);
        }
        return 1;
    }

    static int field_context_newindex(lua_State* lua) {
        return luaL_error(lua, "game.field is native read-only context");
    }

    static int movement_collision(lua_State* lua) {
        auto* self = from_upvalue(lua);
        std::uint32_t x = 0U;
        std::uint32_t y = 0U;
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->movement_collision ||
            !read_integer(lua, 1, 0xFFFFU, x) ||
            !read_integer(lua, 2, 0xFFFFU, y)) {
            return luaL_error(
                lua,
                "game.movement.collision expects x/y coordinates during an actor heartbeat");
        }
        try {
            const auto result =
                self->active_actor_context->movement_collision(
                    static_cast<std::uint16_t>(x),
                    static_cast<std::uint16_t>(y));
            lua_pushinteger(lua, result.collision_code);
            lua_pushinteger(lua, result.terrain_collision_code);
            return 2;
        } catch (...) {
            return luaL_error(lua,
                              "native movement collision query failed");
        }
    }

    static int movement_route(lua_State* lua) {
        auto* self = from_upvalue(lua);
        std::uint32_t route_key = 0U;
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->movement_route_lookup ||
            !read_integer(lua, 1, 12U, route_key)) {
            return luaL_error(
                lua,
                "game.movement.route expects a source route key during an actor heartbeat");
        }
        try {
            const auto route =
                self->active_actor_context->movement_route_lookup(
                    static_cast<std::uint16_t>(route_key));
            lua_createtable(lua, 0, 4);
            lua_pushinteger(lua, route.primary_marker);
            lua_setfield(lua, -2, "primary_marker");
            lua_pushinteger(lua, route.secondary_marker);
            lua_setfield(lua, -2, "secondary_marker");
            lua_pushinteger(lua, route.primary_fallback_slot);
            lua_setfield(lua, -2, "primary_fallback_slot");
            lua_pushinteger(lua, route.secondary_fallback_slot);
            lua_setfield(lua, -2, "secondary_fallback_slot");
            return 1;
        } catch (...) {
            return luaL_error(lua, "native movement route lookup failed");
        }
    }

    static int movement_recover_collision(lua_State* lua) {
        auto* self = from_upvalue(lua);
        std::uint32_t x = 0U;
        std::uint32_t y = 0U;
        std::uint32_t direction_slot = 0U;
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->movement_collision_recovery ||
            !read_integer(lua, 1, 0xFFFFU, x) ||
            !read_integer(lua, 2, 0xFFFFU, y) ||
            !lua_isinteger(lua, 3) || lua_tointeger(lua, 3) >= 0 ||
            lua_tointeger(lua, 3) < -32768 ||
            !read_integer(lua, 4, 14U, direction_slot) ||
            (direction_slot & 1U) != 0U) {
            return luaL_error(
                lua,
                "game.movement.recover_collision expects a negative collision and an even direction slot");
        }
        try {
            const auto recovered =
                self->active_actor_context->movement_collision_recovery(
                    static_cast<std::uint16_t>(x),
                    static_cast<std::uint16_t>(y),
                    static_cast<std::int16_t>(lua_tointeger(lua, 3)),
                    static_cast<std::uint8_t>(direction_slot));
            lua_pushboolean(lua, recovered);
            return 1;
        } catch (...) {
            return luaL_error(
                lua, "native movement collision recovery failed");
        }
    }

    static int movement_projectile_terrain(lua_State* lua) {
        auto* self = from_upvalue(lua);
        std::uint32_t x = 0U;
        std::uint32_t y = 0U;
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->projectile_movement.has_value() ||
            !self->active_actor_context->projectile_terrain_probe ||
            !read_integer(lua, 1, 0xFFFFU, x) ||
            !read_integer(lua, 2, 0xFFFFU, y)) {
            return luaL_error(
                lua,
                "game.movement.projectile_terrain expects x/y during a projectile behavior heartbeat");
        }
        try {
            const auto sample =
                self->active_actor_context->projectile_terrain_probe(
                    static_cast<std::uint16_t>(x),
                    static_cast<std::uint16_t>(y));
            lua_pushinteger(lua, sample.terrain_result);
            lua_pushboolean(lua, sample.terrain_boundary_active);
            lua_pushboolean(lua, sample.outside_bounds_marker);
            return 3;
        } catch (...) {
            return luaL_error(
                lua, "native projectile terrain query failed");
        }
    }

    static int movement_projectile_collision(lua_State* lua) {
        auto* self = from_upvalue(lua);
        std::uint32_t x = 0U;
        std::uint32_t y = 0U;
        std::uint32_t source_flags = 0U;
        if (self == nullptr || self->active_actor_context == nullptr ||
            !self->active_actor_context->projectile_movement.has_value() ||
            !self->active_actor_context->projectile_collision_search ||
            !read_integer(lua, 1, 0xFFFFU, x) ||
            !read_integer(lua, 2, 0xFFFFU, y) ||
            !read_integer(lua, 3, 0xFFU, source_flags)) {
            return luaL_error(
                lua,
                "game.movement.projectile_collision expects x/y/flags during a projectile behavior heartbeat");
        }
        try {
            const auto result =
                self->active_actor_context->projectile_collision_search(
                    static_cast<std::uint16_t>(x),
                    static_cast<std::uint16_t>(y),
                    static_cast<std::uint8_t>(source_flags));
            lua_pushboolean(lua, result.source_carry);
            lua_pushboolean(lua, result.source_position_updated);
            lua_pushinteger(lua, result.source_x);
            lua_pushinteger(lua, result.source_y);
            return 4;
        } catch (...) {
            return luaL_error(
                lua, "native projectile collision search failed");
        }
    }

    static int actor_walk_heading_continuation(
        lua_State*, int, lua_KContext) {
        // The host resumes this continuation with the native service result.
        return 1;
    }

    static int actor_walk_heading(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.actor.walk_heading must run inside an active task";
            }
            lua_pushboolean(lua, 0);
            return 1;
        }

        std::size_t actor_length = 0U;
        const auto* actor = lua_type(lua, 1) == LUA_TSTRING
                                ? lua_tolstring(lua, 1, &actor_length)
                                : nullptr;
        const bool valid_actor = actor != nullptr && actor_length == 6U &&
                                 std::string_view(actor, actor_length) ==
                                     "player";
        const bool valid_heading = lua_isinteger(lua, 2) != 0 &&
                                   lua_tointeger(lua, 2) >= 0 &&
                                   lua_tointeger(lua, 2) <= 7;
        if (!valid_actor || !valid_heading) {
            self->bridge_failure =
                "game.actor.walk_heading expects the player and a heading from 0 to 7";
            lua_pushboolean(lua, 0);
            return 1;
        }

        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::actor_walk_heading;
        operation.actor_id = "player";
        operation.heading = static_cast<std::uint8_t>(lua_tointeger(lua, 2));
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 0, actor_walk_heading_continuation);
    }

    static int ui_menu(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.ui.menu must run inside an active task";
            }
            lua_pushnil(lua);
            return 1;
        }

        std::string menu_id;
        ScriptMenuDefinition menu{};
        if (!read_string(lua, 1, menu_id) ||
            !valid_identifier(menu_id) ||
            !valid_identifier(self->location_text_profile) ||
            !parse_menu_choices(lua, 2, menu_id,
                                self->location_text_profile,
                                self->location_speaker_id, menu,
                                kMaximumMenuChoices)) {
            self->bridge_failure =
                "game.ui.menu expects a menu name and unique stable choice ids in a profiled location";
            lua_pushnil(lua);
            return 1;
        }

        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::ui_menu;
        operation.menu = std::move(menu);
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 1, actor_walk_heading_continuation);
    }

    static int ui_say(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.ui.say must run inside an active task";
            }
            lua_pushboolean(lua, 0);
            return 1;
        }

        std::string event_id;
        if (!read_string(lua, 1, event_id) ||
            !valid_identifier(event_id) ||
            !valid_identifier(self->location_text_profile) ||
            !valid_identifier(self->location_speaker_id)) {
            self->bridge_failure =
                "game.ui.say expects an event in a location with a speaker and text profile";
            lua_pushboolean(lua, 0);
            return 1;
        }

        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::ui_say;
        operation.dialogue = ScriptDialogueRequest{
            self->location_speaker_id, self->location_text_profile,
            std::move(event_id)};
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 1, actor_walk_heading_continuation);
    }

    static int services_call(lua_State* lua) {
        auto* self = from_upvalue(lua);
        if (self == nullptr || self->task_state != lua ||
            self->task_waiting || self->pending_task_operation.has_value() ||
            self->active_native_operation.has_value()) {
            if (self != nullptr) {
                self->bridge_failure =
                    "game.services.call must run inside an active task";
            }
            lua_pushnil(lua);
            return 1;
        }

        std::string domain;
        std::string action;
        if (!read_string(lua, 1, domain) || !valid_identifier(domain) ||
            !read_string(lua, 2, action) || !valid_identifier(action)) {
            self->bridge_failure =
                "game.services.call expects a service domain and action id";
            lua_pushnil(lua);
            return 1;
        }

        ScriptNativeOperation operation{};
        operation.type = ScriptNativeOperation::Type::service_call;
        operation.service_id = domain + "." + action;
        self->pending_task_operation = std::move(operation);
        self->task_waiting = true;
        return lua_yieldk(lua, 0, 1, actor_walk_heading_continuation);
    }

    void release_task() {
        task_waiting = false;
        pending_task_operation.reset();
        active_native_operation.reset();
        bridge_failure.clear();
        if (task_state != nullptr) {
            lua_settop(task_state, 0);
        }
        if (state != nullptr && task_reference != LUA_NOREF) {
            luaL_unref(state, LUA_REGISTRYINDEX, task_reference);
        }
        task_state = nullptr;
        task_reference = LUA_NOREF;
    }

    ScriptTaskStep failed_task(std::string message) {
        failure = std::move(message);
        release_task();
        return {ScriptTaskStep::Status::failed, std::nullopt};
    }

    ScriptTaskStep resume_task(const int argument_count) {
        if (state == nullptr || task_state == nullptr) {
            return failed_task("no active gameplay script task");
        }
        bridge_failure.clear();
        int result_count = 0;
        int status = LUA_ERRRUN;
        {
            ScopedInstructionBudget instruction_budget(task_state);
            status = lua_resume(task_state, state, argument_count,
                                &result_count);
        }
        if (!bridge_failure.empty()) {
            return failed_task(bridge_failure);
        }
        if (status == LUA_YIELD) {
            if (!task_waiting || !pending_task_operation.has_value()) {
                return failed_task(
                    "gameplay script yielded without a native operation");
            }
            ScriptTaskStep step{};
            step.status =
                ScriptTaskStep::Status::awaiting_native_operation;
            active_native_operation = std::move(pending_task_operation);
            pending_task_operation.reset();
            step.operation = active_native_operation;
            failure.clear();
            return step;
        }
        if (status != LUA_OK) {
            return failed_task(stack_error(task_state));
        }
        ScriptTaskStep step{};
        step.status = ScriptTaskStep::Status::completed;
        if (result_count == 1 && lua_type(task_state, -1) == LUA_TSTRING) {
            std::string result_id;
            if (!read_string(task_state, -1, result_id) ||
                !valid_identifier(result_id)) {
                return failed_task(
                    "gameplay script returned an invalid choice id");
            }
            step.result_id = std::move(result_id);
        } else if (result_count > 1 ||
                   (result_count == 1 &&
                    lua_type(task_state, -1) != LUA_TBOOLEAN &&
                    lua_type(task_state, -1) != LUA_TNIL)) {
            return failed_task(
                "gameplay task must return no value, a boolean, or a choice id");
        }
        lua_settop(task_state, 0);
        release_task();
        failure.clear();
        return step;
    }

    ~Impl() {
        if (state != nullptr) {
            lua_close(state);
        }
    }

    void reset() {
        if (state != nullptr) {
            lua_close(state);
        }
        state = luaL_newstate();
        task_state = nullptr;
        modules_reference = LUA_NOREF;
        task_reference = LUA_NOREF;
        task_waiting = false;
        pending_task_operation.reset();
        active_native_operation.reset();
        story_facts.clear();
        dialogue_texts.clear();
        location_type.clear();
        location_id.clear();
        location_text_profile.clear();
        location_speaker_id.clear();
        field_mode_context = ScriptFieldModeContext{};
        active_actor_context = nullptr;
        bridge_failure.clear();
        failure.clear();
        if (state == nullptr) {
            failure = "could not create Lua state";
            return;
        }
        open_safe_libraries(state);
        lua_newtable(state);
        modules_reference = luaL_ref(state, LUA_REGISTRYINDEX);

        lua_newtable(state);
        lua_newtable(state); // game.actors registry; Lua owns actor behavior state
        lua_setfield(state, -2, "actors");
        lua_newtable(state); // game.state proxy
        lua_newtable(state); // game.state metatable
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, state_index, 1);
        lua_setfield(state, -2, "__index");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, state_newindex, 1);
        lua_setfield(state, -2, "__newindex");
        lua_setmetatable(state, -2);
        lua_setfield(state, -2, "state");
        lua_newtable(state); // game.location proxy
        lua_newtable(state); // game.location metatable
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, location_index, 1);
        lua_setfield(state, -2, "__index");
        lua_pushcfunction(state, location_newindex);
        lua_setfield(state, -2, "__newindex");
        lua_setmetatable(state, -2);
        lua_setfield(state, -2, "location");
        lua_newtable(state); // game.field proxy
        lua_newtable(state); // game.field metatable
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, field_context_index, 1);
        lua_setfield(state, -2, "__index");
        lua_pushcfunction(state, field_context_newindex);
        lua_setfield(state, -2, "__newindex");
        lua_setmetatable(state, -2);
        lua_setfield(state, -2, "field");
        lua_newtable(state);
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, actor_walk_heading, 1);
        lua_setfield(state, -2, "walk_heading");
        lua_setfield(state, -2, "actor");
        lua_newtable(state);
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, ui_menu, 1);
        lua_setfield(state, -2, "menu");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, ui_say, 1);
        lua_setfield(state, -2, "say");
        lua_setfield(state, -2, "ui");
        lua_newtable(state);
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, services_call, 1);
        lua_setfield(state, -2, "call");
        lua_setfield(state, -2, "services");
        lua_newtable(state);
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, movement_collision, 1);
        lua_setfield(state, -2, "collision");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, movement_route, 1);
        lua_setfield(state, -2, "route");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, movement_recover_collision, 1);
        lua_setfield(state, -2, "recover_collision");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, movement_projectile_terrain, 1);
        lua_setfield(state, -2, "projectile_terrain");
        lua_pushlightuserdata(state, this);
        lua_pushcclosure(state, movement_projectile_collision, 1);
        lua_setfield(state, -2, "projectile_collision");
        lua_setfield(state, -2, "movement");
        lua_rawgeti(state, LUA_REGISTRYINDEX, modules_reference);
        lua_setfield(state, -2, "modules");
        lua_setglobal(state, "game");
    }

    bool push_module(const std::string_view name) {
        if (state == nullptr || modules_reference == LUA_NOREF ||
            !valid_identifier(name)) {
            return false;
        }
        lua_rawgeti(state, LUA_REGISTRYINDEX, modules_reference);
        lua_getfield(state, -1, std::string(name).c_str());
        lua_remove(state, -2);
        if (!lua_istable(state, -1)) {
            lua_pop(state, 1);
            return false;
        }
        return true;
    }
};

GameScriptRuntime::GameScriptRuntime() : impl_(std::make_unique<Impl>()) {}
GameScriptRuntime::~GameScriptRuntime() = default;
GameScriptRuntime::GameScriptRuntime(GameScriptRuntime&&) noexcept = default;
GameScriptRuntime& GameScriptRuntime::operator=(GameScriptRuntime&&) noexcept = default;

bool GameScriptRuntime::load_package(
    const std::filesystem::path& script_directory,
    const std::filesystem::path& dialogue_text_file) {
    impl_->reset();
    if (impl_->state == nullptr) {
        return false;
    }
    std::error_code error;
    if (!std::filesystem::is_directory(script_directory, error) || error) {
        impl_->failure = "gameplay script directory is missing: " +
                         script_directory.string();
        return false;
    }

    std::vector<std::filesystem::path> files;
    for (std::filesystem::directory_iterator iterator(script_directory, error),
         end;
         !error && iterator != end; iterator.increment(error)) {
        if (iterator->is_regular_file(error) &&
            iterator->path().extension() == ".lua") {
            files.push_back(iterator->path());
        }
    }
    if (error) {
        impl_->failure = "could not enumerate gameplay scripts: " +
                         error.message();
        return false;
    }
    std::sort(files.begin(), files.end());
    if (files.empty() || files.size() > kMaximumScriptModules) {
        impl_->failure = files.empty()
                             ? "gameplay script package has no .lua modules"
                             : "gameplay script package exceeds module limit";
        return false;
    }

    auto* state = impl_->state;
    for (const auto& file : files) {
        const auto module = file.stem().string();
        if (!valid_identifier(module)) {
            impl_->failure = "invalid gameplay script module name: " + module;
            return false;
        }
        ScopedInstructionBudget instruction_budget(state);
        if (luaL_loadfilex(state, file.string().c_str(), "t") != LUA_OK ||
            lua_pcall(state, 0, 1, 0) != LUA_OK) {
            impl_->failure = file.filename().string() + ": " +
                             stack_error(state);
            return false;
        }
        if (!lua_istable(state, -1)) {
            impl_->failure = file.filename().string() +
                             " must return a definition table";
            lua_pop(state, 1);
            return false;
        }
        lua_rawgeti(state, LUA_REGISTRYINDEX, impl_->modules_reference);
        lua_insert(state, -2);
        lua_pushlstring(state, module.data(), module.size());
        lua_insert(state, -2);
        lua_settable(state, -3);
        lua_pop(state, 1);
    }

    if (!dialogue_text_file.empty()) {
        error.clear();
        const auto fail_dialogue = [this, state](const std::string& message) {
            impl_->failure = message;
            lua_settop(state, 0);
            return false;
        };
        if (!std::filesystem::is_regular_file(dialogue_text_file, error) ||
            error) {
            return fail_dialogue(
                "gameplay dialogue text script is missing: " +
                dialogue_text_file.string());
        }
        {
            ScopedInstructionBudget instruction_budget(state);
            if (luaL_loadfilex(state, dialogue_text_file.string().c_str(),
                               "t") != LUA_OK ||
                lua_pcall(state, 0, 1, 0) != LUA_OK) {
                return fail_dialogue(
                    dialogue_text_file.filename().string() + ": " +
                    stack_error(state));
            }
        }
        if (!lua_istable(state, -1)) {
            return fail_dialogue(
                "dialogue text script must return a table");
        }
        lua_getfield(state, -1, "texts");
        if (!lua_istable(state, -1)) {
            return fail_dialogue(
                "dialogue text script must return a texts table");
        }
        const auto texts_table = lua_absindex(state, -1);
        std::unordered_map<std::string, std::string> parsed_texts;
        lua_pushnil(state);
        while (lua_next(state, texts_table) != 0) {
            std::string key;
            const bool valid_key = read_string(state, -2, key);
            const bool string_value = lua_type(state, -1) == LUA_TSTRING;
            std::size_t text_length = 0U;
            const auto* text = string_value
                                   ? lua_tolstring(state, -1, &text_length)
                                   : nullptr;
            // Empty Lua strings represent source item categories that are
            // intentionally blank; missing keys remain distinguishable.
            const bool valid_text = string_value && text != nullptr &&
                                    text_length <= kMaximumDialogueTextBytes;
            if (!valid_key || !valid_identifier(key) || !valid_text ||
                parsed_texts.size() >= kMaximumDialogueTextEntries) {
                lua_pop(state, 2);
                return fail_dialogue(
                    "dialogue text script contains an invalid key or value");
            }
            if (!parsed_texts.emplace(
                    std::move(key), std::string(text, text_length)).second) {
                lua_pop(state, 2);
                return fail_dialogue(
                    "dialogue text script contains a duplicate key");
            }
            lua_pop(state, 1);
        }
        if (parsed_texts.empty()) {
            return fail_dialogue("dialogue text script has no entries");
        }
        impl_->dialogue_texts = std::move(parsed_texts);
        lua_settop(state, 0);
    }
    impl_->failure.clear();
    return true;
}

const std::string* GameScriptRuntime::dialogue_text(
    const std::string_view key) const {
    const auto found = impl_->dialogue_texts.find(std::string(key));
    return found == impl_->dialogue_texts.end() ? nullptr : &found->second;
}

const std::unordered_map<std::string, std::string>&
GameScriptRuntime::dialogue_text_entries() const noexcept {
    return impl_->dialogue_texts;
}

std::optional<std::vector<FieldMapEventRule>>
GameScriptRuntime::map_event_rules() {
    auto* state = impl_->state;
    if (!impl_->push_module("field_events")) {
        impl_->failure = "field_events.lua module is missing";
        return std::nullopt;
    }
    lua_getfield(state, -1, "map_events");
    if (!lua_istable(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "field_events.map_events must be an array";
        return std::nullopt;
    }
    const auto rules_table = lua_absindex(state, -1);
    const auto module_table = lua_absindex(state, -2);
    const auto count = lua_rawlen(state, rules_table);
    if (count == 0U || count > kMaximumMapEventRules) {
        lua_pop(state, 2);
        impl_->failure = "field_events.map_events has invalid rule count";
        return std::nullopt;
    }
    std::vector<FieldMapEventRule> rules;
    rules.reserve(count);
    std::set<std::uint32_t> identifiers;
    for (std::size_t index = 1U; index <= count; ++index) {
        lua_rawgeti(state, rules_table, static_cast<lua_Integer>(index));
        FieldMapEventRule rule{};
        bool valid = parse_event_rule(state, -1, rule) &&
                     identifiers.insert(rule.id).second;
        if (valid &&
            rule.action == FieldMapEventScriptAction::run_lua_handler) {
            lua_getfield(state, module_table, rule.handler.c_str());
            valid = lua_isfunction(state, -1) != 0;
            lua_pop(state, 1);
        }
        lua_pop(state, 1);
        if (!valid) {
            lua_pop(state, 2);
            impl_->failure = "field_events.map_events contains an invalid or duplicate rule";
            return std::nullopt;
        }
        rules.push_back(std::move(rule));
    }
    lua_pop(state, 2);
    impl_->failure.clear();
    return rules;
}

std::optional<ScriptShopDefinition> GameScriptRuntime::shop_definition(
    const std::string_view shop_id) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for shop data";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptShopDefinition> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!valid_identifier(shop_id) || !impl_->push_module("shop")) {
        return fail("shop.lua module or shop identifier is invalid");
    }

    lua_getfield(state, -1, "catalog");
    if (!lua_istable(state, -1)) {
        return fail("shop.catalog must be a table");
    }
    lua_getfield(state, -1, std::string(shop_id).c_str());
    if (!lua_istable(state, -1)) {
        return fail("shop catalog entry is missing");
    }
    const auto definition_index = lua_absindex(state, -1);
    ScriptShopDefinition definition;
    definition.id = std::string(shop_id);
    if (!read_field_string(state, definition_index, "title_key",
                           definition.title_key) ||
        !valid_identifier(definition.title_key)) {
        return fail("shop catalog title key is invalid");
    }
    lua_getfield(state, definition_index, "text_keys");
    if (!lua_istable(state, -1)) {
        return fail("shop catalog text_keys must be a table");
    }
    const auto text_keys_index = lua_absindex(state, -1);
    const auto read_text_key = [state, text_keys_index](
                                   const char* field, std::string& value) {
        return read_field_string(state, text_keys_index, field, value) &&
               valid_identifier(value);
    };
    const bool text_keys_valid =
        read_text_key("universal_clips_title",
                      definition.text_keys.universal_clips_title) &&
        read_text_key("universal_clips_description",
                      definition.text_keys.universal_clips_description) &&
        read_text_key("transaction_prompt",
                      definition.text_keys.transaction_prompt) &&
        read_text_key("no_inventory_slot",
                      definition.text_keys.no_inventory_slot) &&
        read_text_key("insufficient_funds",
                      definition.text_keys.insufficient_funds) &&
        read_text_key("special_action_limit",
                      definition.text_keys.special_action_limit) &&
        read_text_key("protected_sale",
                      definition.text_keys.protected_sale) &&
        read_text_key("sale_quote_heading",
                      definition.text_keys.sale_quote_heading) &&
        read_text_key("purchase_price_heading",
                      definition.text_keys.purchase_price_heading);
    lua_pop(state, 1);
    if (!text_keys_valid) {
        return fail("shop catalog transaction text keys are incomplete");
    }
    lua_getfield(state, definition_index, "groups");
    if (!lua_istable(state, -1)) {
        return fail("shop catalog groups must be an array");
    }
    const auto groups_index = lua_absindex(state, -1);
    const auto group_count = lua_rawlen(state, groups_index);
    if (group_count == 0U || group_count > kMaximumShopGroups) {
        return fail("shop catalog has an invalid group count");
    }

    std::set<std::string> group_ids;
    definition.groups.reserve(group_count);
    for (std::size_t group_index = 1U; group_index <= group_count;
         ++group_index) {
        lua_rawgeti(state, groups_index,
                    static_cast<lua_Integer>(group_index));
        if (!lua_istable(state, -1)) {
            return fail("shop catalog contains a non-table group");
        }
        const auto group_table = lua_absindex(state, -1);
        ScriptShopOfferGroup group;
        if (!read_field_string(state, group_table, "id", group.id) ||
            !valid_identifier(group.id) ||
            !group_ids.insert(group.id).second) {
            return fail("shop catalog contains an invalid or duplicate group id");
        }
        lua_getfield(state, group_table, "title_keys");
        if (!lua_istable(state, -1) || lua_rawlen(state, -1) != 2U) {
            return fail("shop offer group must contain two title keys");
        }
        const auto title_keys_index = lua_absindex(state, -1);
        bool title_keys_valid = true;
        for (std::size_t title_index = 0U;
             title_index < group.title_keys.size(); ++title_index) {
            lua_rawgeti(state, title_keys_index,
                        static_cast<lua_Integer>(title_index + 1U));
            title_keys_valid = read_string(state, -1,
                                           group.title_keys[title_index]) &&
                               valid_identifier(
                                   group.title_keys[title_index]);
            lua_pop(state, 1);
            if (!title_keys_valid) {
                break;
            }
        }
        lua_pop(state, 1);
        if (!title_keys_valid) {
            return fail("shop offer group contains an invalid title key");
        }
        lua_getfield(state, group_table, "offers");
        if (!lua_istable(state, -1)) {
            return fail("shop offer group must contain an offers array");
        }
        const auto offers_index = lua_absindex(state, -1);
        const auto offer_count = lua_rawlen(state, offers_index);
        if (offer_count > kMaximumShopOffers) {
            return fail("shop offer group exceeds the eight-slot panel");
        }
        group.offers.reserve(offer_count);
        for (std::size_t offer_index = 1U; offer_index <= offer_count;
             ++offer_index) {
            lua_rawgeti(state, offers_index,
                        static_cast<lua_Integer>(offer_index));
            if (!lua_istable(state, -1)) {
                return fail("shop offer is not a record table");
            }
            const auto offer_table = lua_absindex(state, -1);
            std::uint32_t type = 0U;
            std::uint32_t value = 0U;
            std::uint32_t quantity = 0U;
            const bool valid =
                read_field_integer(state, offer_table, "type", 0xFFU, type) &&
                read_field_integer(state, offer_table, "value", 0xFFU, value) &&
                read_field_integer(state, offer_table, "quantity", 0xFFU,
                                   quantity);
            lua_pop(state, 1);
            if (!valid) {
                return fail("shop offer has invalid type, value, or quantity");
            }
            group.offers.push_back(ScriptShopOffer{
                static_cast<std::uint8_t>(type),
                static_cast<std::uint8_t>(value),
                static_cast<std::uint8_t>(quantity)});
        }
        lua_pop(state, 1);
        lua_pop(state, 1);
        definition.groups.push_back(std::move(group));
    }

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return definition;
}

std::optional<ScriptItemCatalog> GameScriptRuntime::item_catalog() {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "Lua runtime is unavailable for item data";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptItemCatalog> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("items")) {
        return fail("items.lua module is unavailable");
    }
    const auto module_index = lua_absindex(state, -1);
    ScriptItemCatalog catalog;

    lua_getfield(state, module_index, "catalog");
    if (!lua_istable(state, -1)) {
        return fail("items.catalog must be an array");
    }
    const auto items_index = lua_absindex(state, -1);
    const auto item_count = lua_rawlen(state, items_index);
    if (item_count != kMaximumItemCatalogEntries) {
        return fail("items.catalog does not match the 50 source item IDs");
    }
    catalog.items.reserve(item_count);
    std::set<std::uint8_t> item_codes;
    for (std::size_t index = 1U; index <= item_count; ++index) {
        lua_rawgeti(state, items_index, static_cast<lua_Integer>(index));
        if (!lua_istable(state, -1)) {
            return fail("items.catalog contains a non-record value");
        }
        const auto item_index = lua_absindex(state, -1);
        std::uint32_t code = 0U;
        ScriptItemTextKeys item;
        const bool valid =
            read_field_integer(state, item_index, "code", 50U, code) &&
            code > 0U &&
            read_field_string(state, item_index, "name_key", item.name_key) &&
            valid_identifier(item.name_key) &&
            read_field_string(state, item_index, "category_key",
                              item.category_key) &&
            valid_identifier(item.category_key);
        lua_pop(state, 1);
        if (!valid ||
            !item_codes.insert(static_cast<std::uint8_t>(code)).second) {
            return fail("items.catalog contains an invalid or duplicate ID");
        }
        item.code = static_cast<std::uint8_t>(code);
        catalog.items.push_back(std::move(item));
    }
    lua_pop(state, 1);

    lua_getfield(state, module_index, "detail_text_keys");
    if (!lua_istable(state, -1) ||
        lua_rawlen(state, -1) != kMaximumItemDetailTextEntries) {
        return fail("items.detail_text_keys must contain 32 source entries");
    }
    const auto details_index = lua_absindex(state, -1);
    catalog.detail_text_keys.reserve(kMaximumItemDetailTextEntries);
    for (std::size_t index = 1U;
         index <= kMaximumItemDetailTextEntries; ++index) {
        lua_rawgeti(state, details_index, static_cast<lua_Integer>(index));
        std::string key;
        const bool valid = read_string(state, -1, key) &&
                           valid_identifier(key);
        lua_pop(state, 1);
        if (!valid) {
            return fail("items.detail_text_keys contains an invalid key");
        }
        catalog.detail_text_keys.push_back(std::move(key));
    }
    lua_pop(state, 1);

    lua_getfield(state, module_index, "detail_label_keys");
    if (!lua_istable(state, -1) ||
        lua_rawlen(state, -1) != catalog.detail_label_keys.size()) {
        return fail("items.detail_label_keys must contain three source labels");
    }
    const auto labels_index = lua_absindex(state, -1);
    for (std::size_t index = 0; index < catalog.detail_label_keys.size();
         ++index) {
        lua_rawgeti(state, labels_index, static_cast<lua_Integer>(index + 1U));
        const bool valid = read_string(state, -1,
                                       catalog.detail_label_keys[index]) &&
                           valid_identifier(catalog.detail_label_keys[index]);
        lua_pop(state, 1);
        if (!valid) {
            return fail("items.detail_label_keys contains an invalid key");
        }
    }

    lua_settop(state, stack_base);
    impl_->failure.clear();
    return catalog;
}

void GameScriptRuntime::set_story_state(const StoryFacts& facts) {
    impl_->story_facts.clear();
    for (const auto& [name, value] : facts) {
        if (!valid_identifier(name) ||
            impl_->story_facts.size() >= kMaximumStoryFacts) {
            continue;
        }
        impl_->story_facts.emplace(name, value);
    }
}

void GameScriptRuntime::set_story_fact_accessors(
    StoryFactReader read, StoryFactWriter write) {
    impl_->story_fact_reader = std::move(read);
    impl_->story_fact_writer = std::move(write);
}

StoryFacts GameScriptRuntime::story_state() const {
    return impl_->story_facts;
}

void GameScriptRuntime::set_location_context(
    const std::string_view type, const std::string_view id,
    const std::string_view text_profile,
    const std::string_view speaker_id) {
    if (impl_->state == nullptr || !valid_identifier(type) ||
        !valid_identifier(id) ||
        (!text_profile.empty() && !valid_identifier(text_profile)) ||
        (!speaker_id.empty() && !valid_identifier(speaker_id))) {
        impl_->failure = "invalid gameplay location context";
        return;
    }
    impl_->location_type = std::string(type);
    impl_->location_id = std::string(id);
    impl_->location_text_profile = std::string(text_profile);
    impl_->location_speaker_id = std::string(speaker_id);
}

void GameScriptRuntime::set_field_mode_context(
    const ScriptFieldModeContext context) noexcept {
    impl_->field_mode_context = context;
}

std::optional<ScriptActorBehaviorDecision>
GameScriptRuntime::actor_heartbeat(
    const ScriptActorBehaviorContext& context) {
    auto* state = impl_->state;
    const auto actor_id = context.instance_id.empty()
        ? context.profile_id
        : context.instance_id;
    if (state == nullptr || !valid_identifier(context.profile_id) ||
        !valid_identifier(actor_id)) {
        impl_->failure = "gameplay Lua actor AI state is unavailable";
        return std::nullopt;
    }
    if ((context.action_kind == ScriptActorActionKind::none) !=
        (context.action_code == 0U)) {
        impl_->failure = "gameplay Lua actor action source is inconsistent";
        return std::nullopt;
    }
    if (context.projectile_movement.has_value() &&
        (context.profile_id != "weapon_projectile" ||
         context.phase != ScriptActorHeartbeatPhase::behavior ||
         !context.projectile_terrain_probe ||
         !context.projectile_collision_search)) {
        impl_->failure =
            "gameplay Lua projectile behavior services are unavailable";
        return std::nullopt;
    }
    const auto stack_base = lua_gettop(state);
    const auto fail = [this, state, stack_base](const std::string_view reason)
        -> std::optional<ScriptActorBehaviorDecision> {
        lua_settop(state, stack_base);
        impl_->failure = std::string(reason);
        return std::nullopt;
    };
    if (!impl_->push_module("actor_ai")) {
        return fail("gameplay Lua actor AI module is missing");
    }
    lua_getfield(state, -1, "heartbeat");
    if (!lua_isfunction(state, -1)) {
        return fail("actor_ai.heartbeat must be a Lua function");
    }
    lua_remove(state, -2);
    std::string_view action_kind;
    switch (context.action_kind) {
    case ScriptActorActionKind::none:
        action_kind = "none";
        break;
    case ScriptActorActionKind::item:
        action_kind = "item";
        break;
    case ScriptActorActionKind::spell:
        action_kind = "spell";
        break;
    default:
        return fail("gameplay Lua actor action kind is invalid");
    }
    const std::string_view heartbeat_phase =
        context.phase == ScriptActorHeartbeatPhase::behavior
            ? "behavior"
            : "render";
    std::string_view animation_mode;
    switch (context.animation_mode) {
    case ScriptActorAnimationMode::idle:
        animation_mode = "idle";
        break;
    case ScriptActorAnimationMode::walking:
        animation_mode = "walking";
        break;
    case ScriptActorAnimationMode::attack:
        animation_mode = "attack";
        break;
    default:
        return fail("gameplay Lua actor animation mode is invalid");
    }
    lua_createtable(state, 0, 36);
    lua_pushlstring(state, actor_id.data(), actor_id.size());
    lua_setfield(state, -2, "actor_id");
    lua_pushlstring(state, context.profile_id.data(),
                    context.profile_id.size());
    lua_setfield(state, -2, "profile_id");
    lua_pushinteger(state, context.entity_kind);
    lua_setfield(state, -2, "entity_kind");
    lua_pushlstring(state, action_kind.data(), action_kind.size());
    lua_setfield(state, -2, "action_kind");
    lua_pushinteger(state, context.action_code);
    lua_setfield(state, -2, "action_code");
    lua_pushinteger(state, context.entry_delay);
    lua_setfield(state, -2, "entry_delay");
    lua_pushboolean(state, context.entry_started);
    lua_setfield(state, -2, "entry_started");
    lua_pushboolean(state, context.entry_complete);
    lua_setfield(state, -2, "entry_complete");
    lua_pushinteger(state, context.initial_heading);
    lua_setfield(state, -2, "initial_heading");
    lua_pushboolean(state, context.spawn);
    lua_setfield(state, -2, "spawn");
    lua_pushboolean(state, context.movement_allowed);
    lua_setfield(state, -2, "movement_allowed");
    lua_pushinteger(state, context.controller_nibble);
    lua_setfield(state, -2, "controller_nibble");
    lua_createtable(state, 16, 0);
    for (std::size_t index = 0;
         index < context.controller_heading_table.size(); ++index) {
        lua_pushinteger(state, context.controller_heading_table[index]);
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    lua_setfield(state, -2, "controller_heading_table");
    if (context.movement_heading_override.has_value()) {
        lua_pushinteger(state, *context.movement_heading_override);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "movement_heading_override");
    lua_pushlstring(state, heartbeat_phase.data(), heartbeat_phase.size());
    lua_setfield(state, -2, "phase");
    lua_pushlstring(state, animation_mode.data(), animation_mode.size());
    lua_setfield(state, -2, "animation_mode");
    if (context.animation_direction_override.has_value()) {
        lua_pushinteger(state, *context.animation_direction_override);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "animation_direction_override");
    if (context.animation_accumulator_override.has_value()) {
        lua_pushinteger(state, *context.animation_accumulator_override);
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "animation_accumulator_override");
    if (context.projectile_animation.has_value()) {
        const auto& animation = *context.projectile_animation;
        lua_createtable(state, 0, 7);
        lua_pushinteger(state, animation.selector);
        lua_setfield(state, -2, "selector");
        lua_pushinteger(state, animation.frame_count);
        lua_setfield(state, -2, "frame_count");
        lua_pushinteger(state, animation.frame);
        lua_setfield(state, -2, "frame");
        lua_pushinteger(state, animation.accumulator);
        lua_setfield(state, -2, "accumulator");
        lua_pushinteger(state, animation.increment);
        lua_setfield(state, -2, "increment");
        lua_pushinteger(state, animation.period);
        lua_setfield(state, -2, "period");
        lua_pushboolean(state, animation.finite);
        lua_setfield(state, -2, "finite");
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "projectile_animation");
    if (context.projectile_movement.has_value()) {
        const auto& movement = *context.projectile_movement;
        lua_createtable(state, 0, 12);
        const auto role = movement.role == ScriptActorProjectileRole::primary
                              ? "primary"
                              : "secondary";
        lua_pushstring(state, role);
        lua_setfield(state, -2, "role");
        lua_pushboolean(state, movement.active);
        lua_setfield(state, -2, "active");
        lua_pushinteger(state, movement.flags);
        lua_setfield(state, -2, "flags");
        lua_pushinteger(state, movement.record_flags);
        lua_setfield(state, -2, "record_flags");
        lua_pushinteger(state, movement.world_x);
        lua_setfield(state, -2, "world_x");
        lua_pushinteger(state, movement.world_y);
        lua_setfield(state, -2, "world_y");
        lua_pushinteger(state, movement.tile_offset_x);
        lua_setfield(state, -2, "tile_offset_x");
        lua_pushinteger(state, movement.tile_offset_y);
        lua_setfield(state, -2, "tile_offset_y");
        lua_pushinteger(state, movement.retry_counter);
        lua_setfield(state, -2, "retry_counter");
        lua_pushinteger(state, movement.linked_action_timer);
        lua_setfield(state, -2, "linked_action_timer");
        lua_pushinteger(state, movement.animation_selector);
        lua_setfield(state, -2, "animation_selector");
    } else {
        lua_pushnil(state);
    }
    lua_setfield(state, -2, "projectile_movement");
    lua_pushboolean(
        state,
        context.movement_services_available &&
            static_cast<bool>(context.movement_collision) &&
            static_cast<bool>(context.movement_route_lookup) &&
            static_cast<bool>(context.movement_collision_recovery));
    lua_setfield(state, -2, "movement_services_available");
    lua_createtable(state, 8, 0);
    for (std::size_t index = 0; index < context.movement_offsets.size();
         ++index) {
        const auto& offset = context.movement_offsets[index];
        lua_createtable(state, 2, 0);
        lua_pushinteger(state, offset.x);
        lua_rawseti(state, -2, 1);
        lua_pushinteger(state, offset.y);
        lua_rawseti(state, -2, 2);
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    lua_setfield(state, -2, "movement_offsets");
    lua_createtable(state, 8, 0);
    for (std::size_t index = 0; index < context.movement_idle_selectors.size();
         ++index) {
        lua_pushinteger(state, context.movement_idle_selectors[index]);
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    lua_setfield(state, -2, "movement_idle_selectors");
    lua_createtable(state, 0, 13);
    lua_pushinteger(state, context.movement_state.position_x);
    lua_setfield(state, -2, "position_x");
    lua_pushinteger(state, context.movement_state.position_y);
    lua_setfield(state, -2, "position_y");
    lua_pushinteger(state, context.movement_state.heading);
    lua_setfield(state, -2, "heading");
    lua_pushinteger(state, context.movement_state.animation_heading);
    lua_setfield(state, -2, "animation_heading");
    lua_pushinteger(state, context.movement_state.collision_context);
    lua_setfield(state, -2, "collision_context");
    lua_pushinteger(state, context.movement_state.movement_substate);
    lua_setfield(state, -2, "movement_substate");
    lua_pushinteger(state, context.movement_state.route_status);
    lua_setfield(state, -2, "route_status");
    lua_pushinteger(state, context.movement_state.route_flags);
    lua_setfield(state, -2, "route_flags");
    lua_pushinteger(state, context.movement_state.movement_flags);
    lua_setfield(state, -2, "movement_flags");
    lua_pushinteger(state, context.movement_state.collision_flags);
    lua_setfield(state, -2, "collision_flags");
    lua_pushinteger(state, context.movement_state.event_flags);
    lua_setfield(state, -2, "event_flags");
    lua_pushinteger(state, context.movement_state.collision_extent);
    lua_setfield(state, -2, "collision_extent");
    lua_pushinteger(state, context.movement_state.terrain_collision_code);
    lua_setfield(state, -2, "terrain_collision_code");
    lua_setfield(state, -2, "movement_state");
    lua_pushinteger(state, context.movement_entity_flags);
    lua_setfield(state, -2, "movement_entity_flags");
    lua_pushboolean(state, context.movement_transition_active);
    lua_setfield(state, -2, "movement_transition_active");
    lua_pushinteger(state, context.movement_scene_id);
    lua_setfield(state, -2, "movement_scene_id");
    lua_pushinteger(state, context.movement_collision_mode);
    lua_setfield(state, -2, "movement_collision_mode");
    ScopedInstructionBudget instruction_budget(state);
    impl_->active_actor_context = &context;
    const auto actor_status = lua_pcall(state, 1, 1, 0);
    impl_->active_actor_context = nullptr;
    if (actor_status != LUA_OK) {
        return fail("actor_ai.heartbeat: " + stack_error(state));
    }
    if (!lua_istable(state, -1)) {
        return fail("actor_ai.heartbeat must return an actor snapshot table");
    }

    const auto decision_table = lua_absindex(state, -1);
    ScriptActorBehaviorDecision decision;
    std::string action;
    std::string returned_profile_id;
    std::string returned_actor_id;
    std::string returned_action_kind;
    std::uint32_t returned_action_code = 0U;
    if (!read_field_string(state, decision_table, "actor_id",
                           returned_actor_id) ||
        !read_field_string(state, decision_table, "package_id",
                           decision.package_id) ||
        !valid_identifier(decision.package_id) ||
        !read_field_string(state, decision_table, "profile_id",
                           returned_profile_id) ||
        !read_field_string(state, decision_table, "action_kind",
                           returned_action_kind) ||
        !read_field_integer(state, decision_table, "action_code", 0xFFU,
                            returned_action_code)) {
        return fail("actor_ai.heartbeat returned an incomplete snapshot");
    }
    if (returned_actor_id != actor_id ||
        returned_profile_id != context.profile_id ||
        returned_action_kind != action_kind ||
        returned_action_code != context.action_code) {
        return fail("actor_ai.heartbeat changed source actor identity");
    }
    if (!read_field_string(state, decision_table, "action", action) ||
        !read_field_string(state, decision_table, "animation",
                           decision.animation_id) ||
        !read_field_integer(state, decision_table, "direction", 7U,
                            returned_action_code)) {
        return fail("actor_ai.heartbeat returned an incomplete snapshot");
    }
    decision.direction = static_cast<std::uint8_t>(returned_action_code);
    if (!read_field_integer(state, decision_table, "frame", 15U,
                            returned_action_code)) {
        return fail("actor_ai.heartbeat returned an invalid animation frame");
    }
    decision.animation_frame = static_cast<std::uint8_t>(returned_action_code);
    lua_getfield(state, decision_table, "animation_accumulator");
    if (!lua_isinteger(state, -1)) {
        return fail(
            "actor_ai.heartbeat returned an invalid animation accumulator");
    }
    const auto animation_accumulator = lua_tointeger(state, -1);
    if (animation_accumulator < -32768 || animation_accumulator > 32767) {
        return fail(
            "actor_ai.heartbeat returned an out-of-range animation accumulator");
    }
    decision.animation_accumulator =
        static_cast<std::int16_t>(animation_accumulator);
    lua_pop(state, 1);
    lua_getfield(state, decision_table, "animation_selector");
    if (lua_type(state, -1) != LUA_TNIL) {
        const auto maximum_selector = context.projectile_animation.has_value()
                                          ? 0xFFFF
                                          : 0x07FF;
        if (!lua_isinteger(state, -1) || lua_tointeger(state, -1) < 0 ||
            lua_tointeger(state, -1) > maximum_selector) {
            return fail(
                "actor_ai.heartbeat returned an invalid animation selector");
        }
        decision.animation_selector = static_cast<std::uint16_t>(
            lua_tointeger(state, -1));
    }
    lua_pop(state, 1);
    lua_getfield(state, decision_table, "animation_increment");
    if (lua_type(state, -1) != LUA_TNIL) {
        if (!lua_isinteger(state, -1) || lua_tointeger(state, -1) < -32768 ||
            lua_tointeger(state, -1) > 32767) {
            return fail(
                "actor_ai.heartbeat returned an invalid animation increment");
        }
        decision.animation_increment = static_cast<std::int16_t>(
            lua_tointeger(state, -1));
    }
    lua_pop(state, 1);
    const auto read_boolean_field = [state, decision_table](
                                        const char* field,
                                        bool& value) {
        lua_getfield(state, decision_table, field);
        const bool valid = lua_isboolean(state, -1);
        if (valid) {
            value = lua_toboolean(state, -1) != 0;
        }
        lua_pop(state, 1);
        return valid;
    };
    if (!read_boolean_field("transition_started",
                            decision.transition_started) ||
        !read_boolean_field("transition_complete",
                            decision.transition_complete)) {
        return fail("actor_ai.heartbeat returned invalid transition state");
    }
    if (action == "hold" &&
        (decision.animation_id == "idle" ||
         decision.animation_id == "entry" ||
         decision.animation_id == "walk" ||
         decision.animation_id == "attack" ||
         decision.animation_id == "projectile")) {
        decision.action = ScriptActorBehaviorDecision::Action::hold;
    } else if (action == "play_entry" &&
               decision.animation_id == "entry") {
        decision.action =
            ScriptActorBehaviorDecision::Action::play_entry_animation;
    } else {
        return fail("actor_ai.heartbeat returned an unsupported action");
    }
    // Every phase returns the actor's full state snapshot. A behavior
    // heartbeat may therefore report the walk/attack pose last selected by
    // the render heartbeat without advancing that animation itself.
    if ((decision.animation_id == "walk" ||
         decision.animation_id == "attack") &&
        context.profile_id != "field_runner_k03_none00") {
        return fail("actor_ai.heartbeat returned an invalid player animation");
    }

    lua_getfield(state, decision_table, "audio_event");
    if (lua_type(state, -1) != LUA_TNIL) {
        std::uint32_t event = 0U;
        if (!read_integer(state, -1, 0xFFFFU, event)) {
            return fail("actor_ai.heartbeat returned an invalid audio event");
        }
        decision.audio_event = static_cast<std::uint16_t>(event);
    }
    lua_pop(state, 1);
    lua_getfield(state, decision_table, "weapon_item_id");
    if (lua_type(state, -1) != LUA_TNIL) {
        std::uint32_t item_id = 0U;
        if (!read_integer(state, -1, 50U, item_id) || item_id == 0U ||
            context.action_kind != ScriptActorActionKind::item ||
            item_id != context.action_code) {
            return fail("actor_ai.heartbeat returned an invalid weapon item");
        }
        decision.weapon_item_id = static_cast<std::uint8_t>(item_id);
    }
    lua_pop(state, 1);
    lua_getfield(state, decision_table, "movement_current_heading");
    if (lua_type(state, -1) != LUA_TNIL) {
        std::uint32_t heading = 0U;
        if (!read_integer(state, -1, 0xFFFFU, heading) ||
            context.profile_id != "field_runner_k03_none00") {
            return fail(
                "actor_ai.heartbeat returned an invalid current heading");
        }
        decision.movement_current_heading =
            static_cast<std::uint16_t>(heading);
    }
    lua_pop(state, 1);
    lua_getfield(state, decision_table, "movement_heading");
    if (lua_type(state, -1) != LUA_TNIL) {
        std::uint32_t heading = 0U;
        if (!read_integer(state, -1, 0xFFFEU, heading) ||
            context.profile_id != "field_runner_k03_none00" ||
            context.phase != ScriptActorHeartbeatPhase::behavior ||
            !context.movement_allowed) {
            return fail(
                "actor_ai.heartbeat returned an invalid movement intent");
        }
        decision.movement_heading = static_cast<std::uint16_t>(heading);
    }
    lua_pop(state, 1);
    lua_getfield(state, decision_table, "movement_step");
    if (lua_type(state, -1) != LUA_TNIL) {
        if (!lua_istable(state, -1) ||
            context.profile_id != "field_runner_k03_none00" ||
            context.phase != ScriptActorHeartbeatPhase::behavior ||
            !context.movement_allowed ||
            !context.movement_services_available) {
            return fail("actor_ai.heartbeat returned an invalid movement step");
        }
        const auto movement_table = lua_absindex(state, -1);
        ScriptActorMovementStep movement_step{};
        std::string outcome;
        if (!read_field_string(state, movement_table, "outcome", outcome) ||
            (outcome != "accepted" && outcome != "reset")) {
            return fail("actor_ai.heartbeat returned an invalid movement outcome");
        }
        movement_step.outcome = outcome == "reset"
                                    ? ScriptActorMovementStep::Outcome::reset
                                    : ScriptActorMovementStep::Outcome::accepted;
        lua_getfield(state, movement_table, "state");
        if (!lua_istable(state, -1)) {
            return fail("actor_ai.heartbeat omitted movement state");
        }
        const auto movement_state_table = lua_absindex(state, -1);
        std::uint32_t movement_value = 0U;
        const auto read_movement = [&](const char* field,
                                       const std::uint32_t maximum,
                                       auto& destination) {
            if (!read_field_integer(state, movement_state_table, field,
                                    maximum, movement_value)) {
                return false;
            }
            destination = static_cast<std::remove_reference_t<decltype(destination)>>(
                movement_value);
            return true;
        };
        if (!read_movement("position_x", 0xFFFFU,
                           movement_step.state.position_x) ||
            !read_movement("position_y", 0xFFFFU,
                           movement_step.state.position_y) ||
            !read_movement("heading", 0xFFFFU,
                           movement_step.state.heading) ||
            !read_movement("animation_heading", 0xFFFFU,
                           movement_step.state.animation_heading) ||
            !read_movement("collision_context", 0xFFFFU,
                           movement_step.state.collision_context) ||
            !read_movement("movement_substate", 0xFFFFU,
                           movement_step.state.movement_substate) ||
            !read_movement("route_flags", 0xFFU,
                           movement_step.state.route_flags) ||
            !read_movement("movement_flags", 0xFFU,
                           movement_step.state.movement_flags) ||
            !read_movement("collision_flags", 0xFFU,
                           movement_step.state.collision_flags) ||
            !read_movement("event_flags", 0xFFU,
                           movement_step.state.event_flags)) {
            return fail("actor_ai.heartbeat returned incomplete movement state");
        }
        lua_getfield(state, movement_state_table, "collision_extent");
        if (!lua_isinteger(state, -1) || lua_tointeger(state, -1) < -32768 ||
            lua_tointeger(state, -1) > 32767) {
            return fail("actor_ai.heartbeat returned invalid collision extent");
        }
        movement_step.state.collision_extent =
            static_cast<std::int16_t>(lua_tointeger(state, -1));
        lua_pop(state, 1);
        if (!read_movement("terrain_collision_code", 0xFFFFU,
                           movement_step.state.terrain_collision_code)) {
            return fail("actor_ai.heartbeat returned invalid terrain collision state");
        }
        lua_getfield(state, movement_state_table, "route_status");
        if (!lua_isinteger(state, -1) || lua_tointeger(state, -1) < -32768 ||
            lua_tointeger(state, -1) > 32767) {
            return fail("actor_ai.heartbeat returned invalid movement route status");
        }
        movement_step.state.route_status =
            static_cast<std::int16_t>(lua_tointeger(state, -1));
        lua_pop(state, 1);
        lua_pop(state, 1);
        if (!read_field_integer(state, movement_table, "direction_animation",
                                0xFFFFU, movement_value)) {
            return fail("actor_ai.heartbeat omitted movement animation selector");
        }
        movement_step.direction_animation =
            static_cast<std::uint16_t>(movement_value);
        decision.movement_step = movement_step;
    }
    lua_pop(state, 1);
    if (context.movement_services_available &&
        context.phase == ScriptActorHeartbeatPhase::behavior &&
        context.movement_allowed && decision.movement_heading.has_value() &&
        !decision.movement_step.has_value()) {
        return fail("actor_ai.heartbeat omitted the Lua movement step");
    }
    lua_getfield(state, decision_table, "projectile_movement");
    if (context.projectile_movement.has_value()) {
        if (!lua_istable(state, -1) ||
            context.profile_id != "weapon_projectile" ||
            context.phase != ScriptActorHeartbeatPhase::behavior) {
            return fail(
                "actor_ai.heartbeat returned an invalid projectile movement step");
        }
        const auto movement_table = lua_absindex(state, -1);
        ScriptActorProjectileMovementDecision movement_decision{};
        movement_decision.state = *context.projectile_movement;
        const auto read_boolean = [state, movement_table](
                                      const char* field, bool& value) {
            lua_getfield(state, movement_table, field);
            const bool valid = lua_isboolean(state, -1);
            if (valid) {
                value = lua_toboolean(state, -1) != 0;
            }
            lua_pop(state, 1);
            return valid;
        };
        const auto read_signed = [state, movement_table](
                                     const char* field, const lua_Integer min,
                                     const lua_Integer max,
                                     lua_Integer& value) {
            lua_getfield(state, movement_table, field);
            const bool valid = lua_isinteger(state, -1) &&
                               lua_tointeger(state, -1) >= min &&
                               lua_tointeger(state, -1) <= max;
            if (valid) {
                value = lua_tointeger(state, -1);
            }
            lua_pop(state, 1);
            return valid;
        };
        std::string role;
        std::uint32_t unsigned_value = 0U;
        lua_Integer signed_value = 0;
        if (!read_field_string(state, movement_table, "role", role) ||
            role != (context.projectile_movement->role ==
                             ScriptActorProjectileRole::primary
                         ? "primary"
                         : "secondary") ||
            !read_boolean("active", movement_decision.state.active) ||
            !read_field_integer(state, movement_table, "flags", 0xFFU,
                                unsigned_value)) {
            return fail(
                "actor_ai.heartbeat returned an incomplete projectile state");
        }
        movement_decision.state.flags =
            static_cast<std::uint8_t>(unsigned_value);
        if (!read_field_integer(state, movement_table, "record_flags", 0xFFU,
                                unsigned_value)) {
            return fail(
                "actor_ai.heartbeat returned invalid projectile record flags");
        }
        movement_decision.state.record_flags =
            static_cast<std::uint8_t>(unsigned_value);
        if (!read_signed("world_x",
                         static_cast<lua_Integer>(-2147483647LL - 1LL),
                         2147483647LL, signed_value)) {
            return fail(
                "actor_ai.heartbeat returned invalid projectile X position");
        }
        movement_decision.state.world_x =
            static_cast<std::int32_t>(signed_value);
        if (!read_signed("world_y",
                         static_cast<lua_Integer>(-2147483647LL - 1LL),
                         2147483647LL, signed_value)) {
            return fail(
                "actor_ai.heartbeat returned invalid projectile Y position");
        }
        movement_decision.state.world_y =
            static_cast<std::int32_t>(signed_value);
        if (!read_signed("tile_offset_x", -32768, 32767, signed_value)) {
            return fail(
                "actor_ai.heartbeat returned invalid projectile X offset");
        }
        movement_decision.state.tile_offset_x =
            static_cast<std::int16_t>(signed_value);
        if (!read_signed("tile_offset_y", -32768, 32767, signed_value)) {
            return fail(
                "actor_ai.heartbeat returned invalid projectile Y offset");
        }
        movement_decision.state.tile_offset_y =
            static_cast<std::int16_t>(signed_value);
        if (!read_field_integer(state, movement_table, "retry_counter", 0xFFU,
                                unsigned_value)) {
            return fail(
                "actor_ai.heartbeat returned invalid projectile retry counter");
        }
        movement_decision.state.retry_counter =
            static_cast<std::uint8_t>(unsigned_value);
        if (!read_signed("linked_action_timer", -128, 127, signed_value)) {
            return fail(
                "actor_ai.heartbeat returned invalid projectile action timer");
        }
        movement_decision.state.linked_action_timer =
            static_cast<std::int8_t>(signed_value);
        if (!read_field_integer(state, movement_table, "animation_selector",
                                0xFFFFU, unsigned_value)) {
            return fail(
                "actor_ai.heartbeat returned incomplete projectile policy state");
        }
        movement_decision.state.animation_selector =
            static_cast<std::uint16_t>(unsigned_value);
        std::string outcome;
        if (!read_field_string(state, movement_table, "outcome", outcome)) {
            return fail(
                "actor_ai.heartbeat returned an invalid projectile outcome");
        }
        if (outcome == "none") {
            movement_decision.outcome =
                ScriptActorProjectileOutcome::none;
        } else if (outcome == "stopped") {
            movement_decision.outcome =
                ScriptActorProjectileOutcome::stopped;
        } else if (outcome == "retired") {
            movement_decision.outcome =
                ScriptActorProjectileOutcome::retired;
        } else if (outcome == "hit_resolution_pending") {
            movement_decision.outcome =
                ScriptActorProjectileOutcome::hit_resolution_pending;
        } else if (outcome == "linked_followup_pending") {
            movement_decision.outcome =
                ScriptActorProjectileOutcome::linked_followup_pending;
        } else {
            return fail(
                "actor_ai.heartbeat returned an unsupported projectile outcome");
        }
        decision.projectile_movement = movement_decision;
    } else if (lua_type(state, -1) != LUA_TNIL) {
        return fail(
            "actor_ai.heartbeat returned unexpected projectile movement state");
    }
    lua_pop(state, 1);
    if (context.action_kind == ScriptActorActionKind::item &&
        !decision.weapon_item_id.has_value()) {
        return fail("actor_ai.heartbeat omitted the source item action");
    }
    lua_pop(state, 1);

    const bool begins_entry = decision.action ==
        ScriptActorBehaviorDecision::Action::play_entry_animation;
    const bool projectile_behavior = context.projectile_movement.has_value();
    if (((begins_entry != decision.audio_event.has_value()) &&
         !projectile_behavior) ||
        (projectile_behavior && decision.audio_event.has_value() &&
         *decision.audio_event != 0x003CU) ||
        (decision.projectile_movement.has_value() &&
         decision.projectile_movement->state.role ==
             ScriptActorProjectileRole::secondary &&
         (decision.projectile_movement->outcome ==
              ScriptActorProjectileOutcome::linked_followup_pending ||
          decision.projectile_movement->outcome ==
              ScriptActorProjectileOutcome::hit_resolution_pending)) ||
        (decision.transition_complete && !decision.transition_started) ||
        ((decision.animation_id == "entry") !=
         decision.transition_started) ||
        (decision.animation_id == "idle" &&
         decision.animation_frame != 0U) ||
        (begins_entry && !decision.transition_started)) {
        return fail("actor_ai.heartbeat returned inconsistent actor state");
    }
    impl_->failure.clear();
    return decision;
}

std::optional<ScriptActorBehaviorDecision>
GameScriptRuntime::actor_behavior_decision(
    const ScriptActorBehaviorContext& context) {
    return actor_heartbeat(context);
}

std::optional<std::string> GameScriptRuntime::call_string(
    const std::string_view module, const std::string_view function) {
    auto* state = impl_->state;
    if (!valid_identifier(function)) {
        impl_->failure = "invalid gameplay script callback id";
        return std::nullopt;
    }
    if (!impl_->push_module(module)) {
        impl_->failure = "gameplay script module is missing: " +
                         std::string(module);
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(function).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "script callback is missing: " +
                         std::string(module) + "." + std::string(function);
        return std::nullopt;
    }
    lua_remove(state, -2);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 0, 1, 0) != LUA_OK) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         ": " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }
    std::string result;
    if (!read_string(state, -1, result) || !valid_identifier(result)) {
        lua_pop(state, 1);
        impl_->failure = "script callback must return a known string id";
        return std::nullopt;
    }
    lua_pop(state, 1);
    impl_->failure.clear();
    return result;
}

std::optional<std::vector<std::string>> GameScriptRuntime::string_list(
    const std::string_view module, const std::string_view function) {
    auto* state = impl_->state;
    if (!valid_identifier(function)) {
        impl_->failure = "invalid gameplay script list function";
        return std::nullopt;
    }
    if (!impl_->push_module(module)) {
        impl_->failure = "gameplay script module is missing: " +
                         std::string(module);
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(function).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "script list function is missing: " +
                         std::string(module) + "." + std::string(function);
        return std::nullopt;
    }
    lua_remove(state, -2);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 0, 1, 0) != LUA_OK) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         ": " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " must return an ordered ID list";
        return std::nullopt;
    }

    const auto count = lua_rawlen(state, -1);
    if (count > kMaximumMenuCatalogChoices) {
        lua_pop(state, 1);
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " returned too many IDs";
        return std::nullopt;
    }
    std::vector<std::string> result;
    result.reserve(count);
    std::set<std::string> unique_ids;
    bool valid = true;
    for (std::size_t index = 1U; index <= count; ++index) {
        lua_rawgeti(state, -1, static_cast<lua_Integer>(index));
        std::string id;
        valid = read_string(state, -1, id) && valid_identifier(id) &&
                unique_ids.insert(id).second;
        lua_pop(state, 1);
        if (!valid) {
            break;
        }
        result.push_back(std::move(id));
    }
    lua_pop(state, 1);
    if (!valid) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " returned an invalid, duplicate, or empty ID";
        return std::nullopt;
    }
    impl_->failure.clear();
    return result;
}

ScriptTaskStep GameScriptRuntime::start_task(
    const std::string_view module, const std::string_view function) {
    auto* state = impl_->state;
    if (state == nullptr) {
        impl_->failure = "gameplay Lua state is not available";
        return {ScriptTaskStep::Status::failed, std::nullopt};
    }
    if (impl_->task_state != nullptr) {
        impl_->failure = "a gameplay script task is already active";
        return {ScriptTaskStep::Status::failed, std::nullopt};
    }
    if (!valid_identifier(function)) {
        impl_->failure = "invalid gameplay script task id";
        return {ScriptTaskStep::Status::failed, std::nullopt};
    }
    if (!impl_->push_module(module)) {
        impl_->failure = "gameplay script module is missing: " +
                         std::string(module);
        return {ScriptTaskStep::Status::failed, std::nullopt};
    }
    lua_getfield(state, -1, std::string(function).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "script task handler is missing: " +
                         std::string(module) + "." + std::string(function);
        return {ScriptTaskStep::Status::failed, std::nullopt};
    }
    lua_remove(state, -2);

    impl_->task_state = lua_newthread(state);
    impl_->task_reference = luaL_ref(state, LUA_REGISTRYINDEX);
    lua_xmove(state, impl_->task_state, 1);
    return impl_->resume_task(0);
}

ScriptTaskStep GameScriptRuntime::complete_native_operation(
    const bool succeeded) {
    auto* state = impl_->state;
    auto* task = impl_->task_state;
    if (state == nullptr || task == nullptr || !impl_->task_waiting ||
        impl_->pending_task_operation.has_value() ||
        !impl_->active_native_operation.has_value() ||
        (impl_->active_native_operation->type !=
             ScriptNativeOperation::Type::actor_walk_heading &&
         impl_->active_native_operation->type !=
             ScriptNativeOperation::Type::ui_say)) {
        return impl_->failed_task(
            "gameplay script task is not waiting for a resumable operation");
    }
    impl_->task_waiting = false;
    impl_->active_native_operation.reset();
    lua_pushboolean(task, succeeded ? 1 : 0);
    return impl_->resume_task(1);
}

ScriptTaskStep GameScriptRuntime::complete_native_menu_choice(
    const std::string_view choice_id) {
    auto* state = impl_->state;
    auto* task = impl_->task_state;
    const auto& operation = impl_->active_native_operation;
    if (state == nullptr || task == nullptr || !impl_->task_waiting ||
        impl_->pending_task_operation.has_value() || !operation.has_value() ||
        operation->type != ScriptNativeOperation::Type::ui_menu ||
        !operation->menu.has_value()) {
        return impl_->failed_task(
            "gameplay script task is not waiting for a menu choice");
    }
    const auto selected = std::find_if(
        operation->menu->choices.begin(), operation->menu->choices.end(),
        [choice_id](const ScriptMenuChoice& choice) {
            return choice.id == choice_id;
        });
    if (selected == operation->menu->choices.end()) {
        return impl_->failed_task(
            "native menu returned a choice absent from the Lua menu");
    }
    impl_->task_waiting = false;
    impl_->active_native_operation.reset();
    lua_pushlstring(task, choice_id.data(), choice_id.size());
    return impl_->resume_task(1);
}

ScriptTaskStep GameScriptRuntime::complete_native_service_result(
    const std::string_view result_id) {
    auto* state = impl_->state;
    auto* task = impl_->task_state;
    const auto& operation = impl_->active_native_operation;
    if (state == nullptr || task == nullptr || !impl_->task_waiting ||
        impl_->pending_task_operation.has_value() || !operation.has_value() ||
        operation->type != ScriptNativeOperation::Type::service_call ||
        result_id.empty() || !valid_identifier(result_id)) {
        return impl_->failed_task(
            "gameplay script task is not waiting for a valid service result");
    }
    impl_->task_waiting = false;
    impl_->active_native_operation.reset();
    lua_pushlstring(task, result_id.data(), result_id.size());
    return impl_->resume_task(1);
}

bool GameScriptRuntime::task_waiting_for_native_operation() const noexcept {
    return impl_->task_state != nullptr && impl_->task_waiting &&
           !impl_->pending_task_operation.has_value() &&
           impl_->active_native_operation.has_value();
}

void GameScriptRuntime::cancel_task() noexcept {
    impl_->release_task();
    impl_->failure.clear();
}

std::optional<ScriptStringMap> GameScriptRuntime::string_map(
    const std::string_view module, const std::string_view function) {
    auto* state = impl_->state;
    if (state == nullptr || !valid_identifier(function)) {
        impl_->failure = "invalid gameplay script string-map callback";
        return std::nullopt;
    }
    if (!impl_->push_module(module)) {
        impl_->failure = "gameplay script module is missing: " +
                         std::string(module);
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(function).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " must be a Lua function";
        return std::nullopt;
    }
    lua_remove(state, -2);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 0, 1, 0) != LUA_OK) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         ": " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }

    ScriptStringMap result;
    bool valid = lua_istable(state, -1);
    const auto table_index = lua_absindex(state, -1);
    if (valid) {
        lua_pushnil(state);
        while (lua_next(state, table_index) != 0) {
            std::string key;
            std::string value;
            if (!read_string(state, -2, key) || !valid_identifier(key) ||
                !read_string(state, -1, value) ||
                !valid_identifier(value) ||
                result.size() >= kMaximumStringMapEntries ||
                !result.emplace(std::move(key), std::move(value)).second) {
                lua_pop(state, 2);
                valid = false;
                break;
            }
            lua_pop(state, 1);
        }
    }
    lua_pop(state, 1);
    if (!valid || result.empty()) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " must return a bounded map of string IDs";
        return std::nullopt;
    }
    impl_->failure.clear();
    return result;
}

std::optional<ScriptSpeakerProfile> GameScriptRuntime::speaker_profile(
    const std::string_view speaker_id) {
    auto* state = impl_->state;
    if (state == nullptr || !valid_identifier(speaker_id)) {
        impl_->failure = "invalid gameplay script speaker id";
        return std::nullopt;
    }
    constexpr std::string_view kModule = "dialogue";
    constexpr std::string_view kFunction = "speaker_profile";
    if (!impl_->push_module(kModule)) {
        impl_->failure = "gameplay script module is missing: dialogue";
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(kFunction).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "dialogue.speaker_profile must be a Lua function";
        return std::nullopt;
    }
    lua_remove(state, -2);
    lua_pushlstring(state, speaker_id.data(), speaker_id.size());
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 1, 1, 0) != LUA_OK) {
        impl_->failure = "dialogue.speaker_profile: " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        impl_->failure = "dialogue speaker profile is unavailable: " +
                         std::string(speaker_id);
        return std::nullopt;
    }

    const auto table_index = lua_absindex(state, -1);
    ScriptSpeakerProfile result;
    result.id = speaker_id;
    const auto read_profile_field = [state, table_index](
                                        const char* field,
                                        std::string& value) {
        lua_getfield(state, table_index, field);
        const bool valid = read_string(state, -1, value);
        lua_pop(state, 1);
        return valid;
    };
    const bool fields_valid =
        read_profile_field("name", result.name) &&
        read_profile_field("text_profile", result.text_profile) &&
        read_profile_field("portrait_alias", result.portrait_alias);
    const bool name_valid =
        !result.name.empty() && result.name.size() <= 80U &&
        std::all_of(result.name.begin(), result.name.end(),
                    [](const char character) {
                        return character >= 0x20 && character <= 0x7E;
                    });
    const bool profile_valid =
        valid_identifier(result.text_profile) &&
        valid_identifier(result.portrait_alias);
    lua_pop(state, 1);
    if (!fields_valid || !name_valid || !profile_valid) {
        impl_->failure = "dialogue speaker profile is malformed: " +
                         std::string(speaker_id);
        return std::nullopt;
    }
    impl_->failure.clear();
    return result;
}

std::optional<ScriptDialogueTextKeys>
GameScriptRuntime::dialogue_event_text_keys(
    const std::string_view speaker_id, const std::string_view text_profile,
    const std::string_view event_id) {
    auto* state = impl_->state;
    if (state == nullptr || !valid_identifier(speaker_id) ||
        !valid_identifier(text_profile) || !valid_identifier(event_id)) {
        impl_->failure = "invalid gameplay script dialogue event context";
        return std::nullopt;
    }
    constexpr std::string_view kModule = "dialogue";
    constexpr std::string_view kFunction = "event_text_keys";
    if (!impl_->push_module(kModule)) {
        impl_->failure = "gameplay script module is missing: dialogue";
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(kFunction).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "dialogue.event_text_keys must be a Lua function";
        return std::nullopt;
    }
    lua_remove(state, -2);
    const auto push_string = [state](const std::string_view value) {
        lua_pushlstring(state, value.data(), value.size());
    };
    push_string(speaker_id);
    push_string(text_profile);
    push_string(event_id);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 3, 1, 0) != LUA_OK) {
        impl_->failure = "dialogue.event_text_keys: " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        impl_->failure = "Lua dialogue event is not bound: " +
                         std::string(speaker_id) + "." +
                         std::string(event_id);
        return std::nullopt;
    }

    const auto table_index = lua_absindex(state, -1);
    ScriptDialogueTextKeys result;
    lua_getfield(state, table_index, "title_key");
    const bool title_valid = read_string(state, -1, result.title_key) &&
                             valid_identifier(result.title_key);
    lua_pop(state, 1);
    lua_getfield(state, table_index, "continuation_key");
    std::size_t continuation_length = 0U;
    const auto* continuation =
        lua_type(state, -1) == LUA_TSTRING
            ? lua_tolstring(state, -1, &continuation_length)
            : nullptr;
    const bool continuation_valid =
        continuation != nullptr &&
        continuation_length <= kMaximumIdentifierLength &&
        (continuation_length == 0U ||
         valid_identifier(std::string_view(continuation,
                                           continuation_length)));
    if (continuation_valid) {
        result.continuation_key.assign(continuation, continuation_length);
    }
    lua_pop(state, 1);
    lua_pop(state, 1);
    if (!title_valid || !continuation_valid) {
        impl_->failure = "Lua dialogue event keys are malformed: " +
                         std::string(speaker_id) + "." +
                         std::string(event_id);
        return std::nullopt;
    }
    impl_->failure.clear();
    return result;
}

std::optional<ScriptMenuDefinition> GameScriptRuntime::resolve_dialogue_menu(
    ScriptMenuDefinition menu) {
    auto* state = impl_->state;
    if (state == nullptr ||
        (!menu.speaker_id.empty() && !valid_identifier(menu.speaker_id)) ||
        !valid_identifier(menu.text_profile) || !valid_identifier(menu.id) ||
        menu.choices.size() > kMaximumMenuChoices) {
        impl_->failure = "invalid gameplay script dialogue menu context";
        return std::nullopt;
    }
    for (const auto& choice : menu.choices) {
        if (!valid_identifier(choice.id)) {
            impl_->failure =
                "dialogue menu contains an invalid stable choice id";
            return std::nullopt;
        }
    }

    constexpr std::string_view kModule = "dialogue";
    constexpr std::string_view kFunction = "menu_text_keys";
    if (!impl_->push_module(kModule)) {
        impl_->failure = "gameplay script module is missing: dialogue";
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(kFunction).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = "dialogue.menu_text_keys must be a Lua function";
        return std::nullopt;
    }
    lua_remove(state, -2);
    const auto push_string = [state](const std::string_view value) {
        lua_pushlstring(state, value.data(), value.size());
    };
    push_string(menu.speaker_id);
    push_string(menu.text_profile);
    push_string(menu.id);
    lua_createtable(state, static_cast<int>(menu.choices.size()), 0);
    for (std::size_t index = 0; index < menu.choices.size(); ++index) {
        const auto& choice_id = menu.choices[index].id;
        lua_pushlstring(state, choice_id.data(), choice_id.size());
        lua_rawseti(state, -2, static_cast<lua_Integer>(index + 1U));
    }
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 4, 1, 0) != LUA_OK) {
        impl_->failure = "dialogue.menu_text_keys: " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }
    if (!lua_istable(state, -1)) {
        lua_pop(state, 1);
        impl_->failure = "Lua dialogue menu is not bound: " +
                         std::string(menu.speaker_id) + "." + menu.id;
        return std::nullopt;
    }

    const auto table_index = lua_absindex(state, -1);
    lua_getfield(state, table_index, "title_key");
    const bool title_valid = read_string(state, -1, menu.title_key) &&
                             valid_identifier(menu.title_key);
    lua_pop(state, 1);
    lua_getfield(state, table_index, "choice_label_keys");
    bool labels_valid = lua_istable(state, -1) &&
                       lua_rawlen(state, -1) == menu.choices.size();
    if (labels_valid) {
        const auto labels_index = lua_absindex(state, -1);
        for (std::size_t index = 0; index < menu.choices.size(); ++index) {
            auto& label_key = menu.choices[index].label_key;
            lua_rawgeti(state, labels_index,
                        static_cast<lua_Integer>(index + 1U));
            labels_valid = read_string(state, -1, label_key) &&
                           valid_identifier(label_key);
            lua_pop(state, 1);
            if (!labels_valid) {
                break;
            }
        }
    }
    lua_pop(state, 2);
    if (!title_valid || !labels_valid) {
        impl_->failure = "Lua dialogue menu keys do not match its choices: " +
                         std::string(menu.speaker_id) + "." + menu.id;
        return std::nullopt;
    }
    impl_->failure.clear();
    return menu;
}

std::optional<ScriptMenuDefinition> GameScriptRuntime::menu(
    const std::string_view module, const std::string_view function,
    const std::string_view text_profile, const std::string_view menu_id) {
    auto* state = impl_->state;
    if (!valid_identifier(function) || !valid_identifier(text_profile) ||
        !valid_identifier(menu_id)) {
        impl_->failure = "invalid gameplay script menu function or profile";
        return std::nullopt;
    }
    if (!impl_->push_module(module)) {
        impl_->failure = "gameplay script module is missing: " +
                         std::string(module);
        return std::nullopt;
    }
    lua_getfield(state, -1, std::string(function).c_str());
    if (!lua_isfunction(state, -1)) {
        lua_pop(state, 2);
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " must be a Lua function";
        return std::nullopt;
    }
    lua_remove(state, -2);
    ScopedInstructionBudget instruction_budget(state);
    if (lua_pcall(state, 0, 3, 0) != LUA_OK) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         ": " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }

    ScriptMenuDefinition definition;
    if (!parse_menu_choices(state, -3, menu_id, text_profile,
                            impl_->location_speaker_id, definition,
                            kMaximumMenuCatalogChoices)) {
        lua_pop(state, 3);
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " must return unique choice ids";
        return std::nullopt;
    }
    if (!parse_explicit_menu_text_keys(state, -2, -1, definition)) {
        lua_pop(state, 3);
        impl_->failure = std::string(module) + "." +
                         std::string(function) +
                         " must return explicit text keys matching its choices";
        return std::nullopt;
    }
    lua_pop(state, 3);
    impl_->failure.clear();
    return definition;
}

const std::string& GameScriptRuntime::failure() const noexcept {
    return impl_->failure;
}

} // namespace shadowrun::ghidra_conversion::scripting
