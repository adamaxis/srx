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
#include <utility>

namespace shadowrun::ghidra_conversion::scripting {
namespace {

constexpr std::size_t kMaximumScriptModules = 128U;
constexpr std::size_t kMaximumMapEventRules = 512U;
constexpr std::size_t kMaximumMenuChoices = 3U;
constexpr std::size_t kMaximumIdentifierLength = 96U;
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
    if (name == "start_new_game_opening_walk") {
        action = FieldMapEventScriptAction::start_new_game_opening_walk;
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
    rule.scene_id = static_cast<std::uint16_t>(scene_id);
    return true;
}

bool parse_menu_choices(lua_State* state, const int index,
                        const std::string_view function,
                        const std::string_view text_profile,
                        ScriptMenuDefinition& menu) {
    if (!lua_istable(state, index) || !valid_identifier(function) ||
        !valid_identifier(text_profile)) {
        return false;
    }
    const auto count = lua_rawlen(state, index);
    if (count > kMaximumMenuChoices) {
        return false;
    }
    menu.id = std::string(function);
    menu.title_key = std::string(text_profile) + ".manager.title";
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
        choice.label_key = std::string(text_profile) + ".option." + choice.id;
        menu.choices.push_back(std::move(choice));
    }
    return true;
}

} // namespace

struct GameScriptRuntime::Impl {
    lua_State* state = nullptr;
    int modules_reference = LUA_NOREF;
    std::string failure;

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
        modules_reference = LUA_NOREF;
        failure.clear();
        if (state == nullptr) {
            failure = "could not create Lua state";
            return;
        }
        open_safe_libraries(state);
        lua_newtable(state);
        modules_reference = luaL_ref(state, LUA_REGISTRYINDEX);

        lua_newtable(state);
        lua_newtable(state);
        lua_setfield(state, -2, "state");
        lua_newtable(state);
        lua_setfield(state, -2, "location");
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
    const std::filesystem::path& script_directory) {
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
    return true;
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
        const bool valid = parse_event_rule(state, -1, rule) &&
                           identifiers.insert(rule.id).second;
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

void GameScriptRuntime::set_story_state(const StoryFacts& facts) {
    auto* state = impl_->state;
    if (state == nullptr) {
        return;
    }
    lua_getglobal(state, "game");
    lua_newtable(state);
    for (const auto& [name, value] : facts) {
        if (!valid_identifier(name)) {
            continue;
        }
        lua_pushboolean(state, value ? 1 : 0);
        lua_setfield(state, -2, name.c_str());
    }
    lua_setfield(state, -2, "state");
    lua_pop(state, 1);
}

StoryFacts GameScriptRuntime::story_state() const {
    StoryFacts facts;
    auto* state = impl_->state;
    if (state == nullptr) {
        return facts;
    }
    lua_getglobal(state, "game");
    lua_getfield(state, -1, "state");
    if (lua_istable(state, -1)) {
        lua_pushnil(state);
        while (lua_next(state, -2) != 0) {
            if (lua_type(state, -2) == LUA_TSTRING &&
                lua_type(state, -1) == LUA_TBOOLEAN) {
                std::size_t length = 0U;
                const auto* name = lua_tolstring(state, -2, &length);
                if (name != nullptr && length > 0U &&
                    length <= kMaximumIdentifierLength) {
                    facts.emplace(std::string(name, length),
                                  lua_toboolean(state, -1) != 0);
                }
            }
            lua_pop(state, 1);
        }
    }
    lua_pop(state, 2);
    return facts;
}

void GameScriptRuntime::set_location_context(const std::string_view type,
                                            const std::string_view id) {
    auto* state = impl_->state;
    if (state == nullptr || !valid_identifier(type) || !valid_identifier(id)) {
        impl_->failure = "invalid gameplay location context";
        return;
    }
    lua_getglobal(state, "game");
    lua_newtable(state);
    lua_pushlstring(state, type.data(), type.size());
    lua_setfield(state, -2, "type");
    lua_pushlstring(state, id.data(), id.size());
    lua_setfield(state, -2, "id");
    lua_setfield(state, -2, "location");
    lua_pop(state, 1);
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

std::optional<ScriptMenuDefinition> GameScriptRuntime::menu(
    const std::string_view module, const std::string_view function,
    const std::string_view text_profile) {
    auto* state = impl_->state;
    if (!valid_identifier(function) || !valid_identifier(text_profile)) {
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
    if (lua_pcall(state, 0, 1, 0) != LUA_OK) {
        impl_->failure = std::string(module) + "." + std::string(function) +
                         ": " + stack_error(state);
        lua_pop(state, 1);
        return std::nullopt;
    }

    ScriptMenuDefinition definition;
    if (!parse_menu_choices(state, -1, function, text_profile, definition)) {
        lua_pop(state, 1);
        impl_->failure = std::string(module) + "." + std::string(function) +
                         " must return unique choice ids";
        return std::nullopt;
    }
    lua_pop(state, 1);
    impl_->failure.clear();
    return definition;
}

const std::string& GameScriptRuntime::failure() const noexcept {
    return impl_->failure;
}

} // namespace shadowrun::ghidra_conversion::scripting
