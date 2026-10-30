#include "shadowrun_ghidra/scripting/cyberdeck_menu_bindings.hpp"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include <algorithm>
#include <array>
#include <cstdint>
#include <optional>
#include <string_view>
#include <type_traits>

namespace shadowrun::ghidra_conversion::scripting {
namespace {

int integer_argument(lua_State* lua, const int index) {
    int is_number = 0;
    const auto value = lua_tointegerx(lua, index, &is_number);
    return is_number ? static_cast<int>(value) : 0;
}

void push_record(lua_State* lua, const ScriptCyberdeckRecord& record) {
    lua_createtable(lua, 0, 12);
    const auto record_index = lua_absindex(lua, -1);
    lua_pushinteger(lua, record.mpcp);
    lua_setfield(lua, record_index, "mpcp");
    lua_pushinteger(lua, record.hardening);
    lua_setfield(lua, record_index, "hardening");
    lua_pushinteger(lua, record.memory);
    lua_setfield(lua, record_index, "memory");
    lua_pushinteger(lua, record.storage);
    lua_setfield(lua, record_index, "storage");
    lua_pushinteger(lua, record.load_io);
    lua_setfield(lua, record_index, "load_io");
    lua_pushinteger(lua, record.response);
    lua_setfield(lua, record_index, "response");
    lua_pushinteger(lua, record.persona_condition);
    lua_setfield(lua, record_index, "persona_condition");
    lua_pushinteger(lua, record.model);
    lua_setfield(lua, record_index, "model");

    const auto push_array = [lua, record_index](const char* name,
                                                 const auto& values) {
        lua_createtable(lua, static_cast<int>(values.size()), 0);
        const auto array_index = lua_absindex(lua, -1);
        for (std::size_t index = 0; index < values.size(); ++index) {
            lua_pushinteger(lua, values[index]);
            lua_rawseti(lua, array_index, static_cast<lua_Integer>(index));
        }
        lua_setfield(lua, record_index, name);
    };
    push_array("persona", record.persona);
    push_array("ratings", record.ratings);
    push_array("active", record.active_programs);

    lua_createtable(lua, static_cast<int>(record.files.size()), 0);
    const auto files_index = lua_absindex(lua, -1);
    for (std::size_t index = 0; index < record.files.size(); ++index) {
        lua_createtable(lua, 0, 3);
        lua_pushinteger(lua, record.files[index].id);
        lua_setfield(lua, -2, "id");
        lua_pushinteger(lua, record.files[index].size);
        lua_setfield(lua, -2, "size");
        lua_pushboolean(lua, record.files[index].value == -1);
        lua_setfield(lua, -2, "run_objective");
        lua_pushinteger(lua, record.files[index].value == -1
                                ? 0 : record.files[index].value);
        lua_setfield(lua, -2, "data_value");
        lua_rawseti(lua, files_index, static_cast<lua_Integer>(index));
    }
    lua_setfield(lua, record_index, "files");
}

int read_integer(lua_State* lua, const int table_index, const char* name,
                 const lua_Integer fallback, const lua_Integer minimum,
                 const lua_Integer maximum) {
    lua_getfield(lua, table_index, name);
    int is_number = 0;
    const auto value = lua_tointegerx(lua, -1, &is_number);
    lua_pop(lua, 1);
    return static_cast<int>(std::clamp<lua_Integer>(
        is_number ? value : fallback, minimum, maximum));
}

std::string_view matrix_security_color_name(
    const ScriptMatrixSecurityColor color) {
    switch (color) {
    case ScriptMatrixSecurityColor::unvisited:
        return "unvisited";
    case ScriptMatrixSecurityColor::blue:
        return "blue";
    case ScriptMatrixSecurityColor::green:
        return "green";
    case ScriptMatrixSecurityColor::orange:
        return "orange";
    case ScriptMatrixSecurityColor::red:
        return "red";
    }
    return "unvisited";
}

ScriptMatrixSecurityColor matrix_security_color_from_lua(
    lua_State* lua, const int table_index) {
    lua_getfield(lua, table_index, "security_color");
    ScriptMatrixSecurityColor color = ScriptMatrixSecurityColor::unvisited;
    if (lua_type(lua, -1) == LUA_TSTRING) {
        std::size_t length = 0U;
        const char* value = lua_tolstring(lua, -1, &length);
        if (value != nullptr) {
            const std::string_view name(value, length);
            if (name == "blue") {
                color = ScriptMatrixSecurityColor::blue;
            } else if (name == "green") {
                color = ScriptMatrixSecurityColor::green;
            } else if (name == "orange") {
                color = ScriptMatrixSecurityColor::orange;
            } else if (name == "red") {
                color = ScriptMatrixSecurityColor::red;
            }
        }
    }
    lua_pop(lua, 1);
    return color;
}

const char* matrix_run_status_name(const ScriptMatrixRunStatus status) {
    switch (status) {
    case ScriptMatrixRunStatus::pending:
        return "pending";
    case ScriptMatrixRunStatus::inactive:
        return "inactive";
    case ScriptMatrixRunStatus::active:
        return "active";
    case ScriptMatrixRunStatus::completed:
        return "completed";
    }
    return "pending";
}

const char* matrix_run_node_name(const ScriptMatrixRunNode node) {
    switch (node) {
    case ScriptMatrixRunNode::cpu:
        return "cpu";
    case ScriptMatrixRunNode::datastore:
        return "datastore";
    }
    return "datastore";
}

const char* matrix_run_action_name(const ScriptMatrixRunAction action) {
    switch (action) {
    case ScriptMatrixRunAction::crash_system:
        return "crash_system";
    case ScriptMatrixRunAction::delete_file:
        return "delete_file";
    case ScriptMatrixRunAction::upload_file:
        return "upload_file";
    case ScriptMatrixRunAction::download_file:
        return "download_file";
    }
    return "download_file";
}

std::optional<ScriptMatrixRunStatus> matrix_run_status_from_lua(
    lua_State* lua, const int table_index) {
    lua_getfield(lua, table_index, "status");
    std::optional<ScriptMatrixRunStatus> status;
    if (lua_type(lua, -1) == LUA_TSTRING) {
        std::size_t length = 0U;
        const char* value = lua_tolstring(lua, -1, &length);
        if (value != nullptr) {
            const std::string_view name(value, length);
            if (name == "pending") {
                status = ScriptMatrixRunStatus::pending;
            } else if (name == "inactive") {
                status = ScriptMatrixRunStatus::inactive;
            } else if (name == "active") {
                status = ScriptMatrixRunStatus::active;
            } else if (name == "completed") {
                status = ScriptMatrixRunStatus::completed;
            }
        }
    }
    lua_pop(lua, 1);
    return status;
}

template <typename Values>
void read_array(lua_State* lua, const int table_index, const char* name,
                Values& values, const int minimum, const int maximum) {
    lua_getfield(lua, table_index, name);
    if (lua_istable(lua, -1)) {
        const auto array_index = lua_absindex(lua, -1);
        for (std::size_t index = 0; index < values.size(); ++index) {
            lua_rawgeti(lua, array_index, static_cast<lua_Integer>(index));
            int is_number = 0;
            const auto value = lua_tointegerx(lua, -1, &is_number);
            if (is_number) {
                values[index] = static_cast<typename Values::value_type>(
                    std::clamp<lua_Integer>(value, minimum, maximum));
            }
            lua_pop(lua, 1);
        }
    }
    lua_pop(lua, 1);
}

ScriptCyberdeckRecord read_record(lua_State* lua, const int table_index) {
    ScriptCyberdeckRecord record;
    record.mpcp = static_cast<std::uint8_t>(
        read_integer(lua, table_index, "mpcp", 0, 0, 0xFF));
    record.hardening = static_cast<std::uint8_t>(
        read_integer(lua, table_index, "hardening", 0, 0, 0xFF));
    record.memory = static_cast<std::uint16_t>(
        read_integer(lua, table_index, "memory", 0, 0, 0xFFFF));
    record.storage = static_cast<std::uint16_t>(
        read_integer(lua, table_index, "storage", 0, 0, 0xFFFF));
    record.load_io = static_cast<std::uint8_t>(
        read_integer(lua, table_index, "load_io", 0, 0, 0xFF));
    record.response = static_cast<std::uint8_t>(
        read_integer(lua, table_index, "response", 0, 0, 0xFF));
    record.persona_condition = static_cast<std::uint8_t>(
        read_integer(lua, table_index, "persona_condition", 0, 0, 0xFF));
    record.model = static_cast<std::int8_t>(
        read_integer(lua, table_index, "model", -1, -128, 127));
    read_array(lua, table_index, "persona", record.persona, 0, 0xFF);
    read_array(lua, table_index, "ratings", record.ratings, 0, 0xFF);
    read_array(lua, table_index, "active", record.active_programs, -1, 11);

    lua_getfield(lua, table_index, "files");
    if (lua_istable(lua, -1)) {
        const auto files_index = lua_absindex(lua, -1);
        for (std::size_t index = 0; index < record.files.size(); ++index) {
            lua_rawgeti(lua, files_index, static_cast<lua_Integer>(index));
            if (lua_istable(lua, -1)) {
                const auto file_index = lua_absindex(lua, -1);
                record.files[index].id = static_cast<std::int8_t>(
                    read_integer(lua, file_index, "id", -1, -1, 127));
                record.files[index].size = static_cast<std::uint16_t>(
                    read_integer(lua, file_index, "size", 0, 0, 0xFFFF));
                lua_getfield(lua, file_index, "run_objective");
                const bool run_objective = lua_type(lua, -1) == LUA_TBOOLEAN &&
                    lua_toboolean(lua, -1) != 0;
                lua_pop(lua, 1);
                record.files[index].value = run_objective
                    ? static_cast<std::int8_t>(-1)
                    : static_cast<std::int8_t>(read_integer(
                        lua, file_index, "data_value", 0, -128, 127));
            }
            lua_pop(lua, 1);
        }
    }
    lua_pop(lua, 1);
    return record;
}

std::optional<lua_Integer> read_optional_integer(
    lua_State* lua, const int table_index, const char* name) {
    lua_getfield(lua, table_index, name);
    int is_number = 0;
    const auto value = lua_tointegerx(lua, -1, &is_number);
    lua_pop(lua, 1);
    if (is_number == 0) {
        return std::nullopt;
    }
    return value;
}

std::optional<bool> read_optional_boolean(
    lua_State* lua, const int table_index, const char* name) {
    lua_getfield(lua, table_index, name);
    const bool is_boolean = lua_isboolean(lua, -1) != 0;
    const bool value = lua_toboolean(lua, -1) != 0;
    lua_pop(lua, 1);
    return is_boolean ? std::optional<bool>{value} : std::nullopt;
}

bool read_boolean(lua_State* lua, const int table_index, const char* name,
                  bool& value) {
    lua_getfield(lua, table_index, name);
    const bool valid = lua_isboolean(lua, -1) != 0;
    if (valid) {
        value = lua_toboolean(lua, -1) != 0;
    }
    lua_pop(lua, 1);
    return valid;
}

std::optional<ScriptActorRoleFacts> read_actor_roles(
    lua_State* lua, const int actor_index) {
    lua_getfield(lua, actor_index, "actor_facts");
    if (!lua_istable(lua, -1)) {
        lua_pop(lua, 1);
        return std::nullopt;
    }
    const auto facts_index = lua_absindex(lua, -1);
    lua_getfield(lua, facts_index, "roles");
    if (!lua_istable(lua, -1)) {
        lua_pop(lua, 2);
        return std::nullopt;
    }
    const auto roles_index = lua_absindex(lua, -1);
    ScriptActorRoleFacts roles;
    const bool valid =
        read_boolean(lua, roles_index, "spotted", roles.spotted) &&
        read_boolean(lua, roles_index, "reinforcement",
                     roles.reinforcement) &&
        read_boolean(lua, roles_index, "despawn_offscreen",
                     roles.despawn_offscreen) &&
        read_boolean(lua, roles_index, "quest", roles.quest) &&
        read_boolean(lua, roles_index, "runner", roles.runner) &&
        read_boolean(lua, roles_index, "pedestrian", roles.pedestrian) &&
        read_boolean(lua, roles_index, "hostile", roles.hostile);
    lua_pop(lua, 2);
    return valid ? std::optional<ScriptActorRoleFacts>{roles}
                 : std::nullopt;
}

void push_actor_roles(lua_State* lua, const int actor_index,
                      const ScriptActorRoleFacts& roles) {
    lua_createtable(lua, 0, 1);
    const auto facts_index = lua_absindex(lua, -1);
    lua_createtable(lua, 0, 7);
    const auto roles_index = lua_absindex(lua, -1);
    const auto push = [lua, roles_index](const char* name,
                                         const bool value) {
        lua_pushboolean(lua, value);
        lua_setfield(lua, roles_index, name);
    };
    push("spotted", roles.spotted);
    push("reinforcement", roles.reinforcement);
    push("despawn_offscreen", roles.despawn_offscreen);
    push("quest", roles.quest);
    push("runner", roles.runner);
    push("pedestrian", roles.pedestrian);
    push("hostile", roles.hostile);
    lua_setfield(lua, facts_index, "roles");
    lua_setfield(lua, actor_index, "actor_facts");
}

void push_optional_boolean(lua_State* lua, const int table_index,
                           const char* name,
                           const std::optional<bool> value) {
    if (!value.has_value()) {
        return;
    }
    lua_pushboolean(lua, *value ? 1 : 0);
    lua_setfield(lua, table_index, name);
}

template <typename Integer>
void push_optional_integer(lua_State* lua, const int table_index,
                           const char* name,
                           const std::optional<Integer>& value) {
    if (!value.has_value()) {
        return;
    }
    lua_pushinteger(lua, static_cast<lua_Integer>(*value));
    lua_setfield(lua, table_index, name);
}

void push_matrix_world_state(lua_State* lua,
                             const ScriptMatrixWorldState& world) {
    lua_createtable(lua, 0, 7);
    const int world_index = lua_absindex(lua, -1);
    push_optional_integer(lua, world_index, "karma_counter",
                          world.karma_counter);
    push_optional_integer(lua, world_index, "passive_ic_counter",
                          world.passive_ic_counter);
    if (world.map_knowledge.has_value()) {
        const auto& knowledge = *world.map_knowledge;
        lua_createtable(lua, 0, 3);
        const auto knowledge_index = lua_absindex(lua, -1);
        lua_pushinteger(lua, knowledge.system_id);
        lua_setfield(lua, knowledge_index, "system_id");
        lua_createtable(lua, static_cast<int>(knowledge.rows.size()), 0);
        const auto rows_index = lua_absindex(lua, -1);
        for (std::size_t y = 0; y < knowledge.rows.size(); ++y) {
            const auto& row = knowledge.rows[y];
            lua_createtable(lua, static_cast<int>(row.size()), 0);
            const auto row_index = lua_absindex(lua, -1);
            for (std::size_t x = 0; x < row.size(); ++x) {
                const auto& cell = row[x];
                lua_createtable(lua, 0, 3);
                lua_pushinteger(lua, cell.analysis_level);
                lua_setfield(lua, -2, "analysis_level");
                const auto color =
                    matrix_security_color_name(cell.security_color);
                lua_pushlstring(lua, color.data(), color.size());
                lua_setfield(lua, -2, "security_color");
                lua_pushboolean(lua, cell.revealed);
                lua_setfield(lua, -2, "revealed");
                lua_rawseti(lua, row_index, static_cast<lua_Integer>(x));
            }
            lua_rawseti(lua, rows_index, static_cast<lua_Integer>(y));
        }
        lua_setfield(lua, knowledge_index, "cells");
        lua_setfield(lua, world_index, "map_knowledge");
    }
    if (world.run.has_value()) {
        const auto& run = *world.run;
        lua_createtable(lua, 0, 6);
        const int run_index = lua_absindex(lua, -1);
        lua_pushstring(lua, matrix_run_status_name(run.status));
        lua_setfield(lua, run_index, "status");
        lua_createtable(lua, 0, 4);
        const int objective_index = lua_absindex(lua, -1);
        lua_pushstring(lua, matrix_run_node_name(run.objective.node));
        lua_setfield(lua, objective_index, "node");
        lua_pushstring(lua, matrix_run_action_name(run.objective.action));
        lua_setfield(lua, objective_index, "action");
        if (!run.objective.file_class.empty()) {
            lua_pushlstring(lua, run.objective.file_class.data(),
                            run.objective.file_class.size());
            lua_setfield(lua, objective_index, "file_class");
        }
        if (run.objective.file_id.has_value()) {
            lua_pushinteger(lua, *run.objective.file_id);
            lua_setfield(lua, objective_index, "file_id");
        }
        lua_setfield(lua, run_index, "objective");
        lua_pushinteger(lua, run.file_size);
        lua_setfield(lua, run_index, "file_size");
        lua_pushboolean(lua, run.completion_marker_present);
        lua_setfield(lua, run_index, "completion_marker_present");
        lua_pushboolean(lua, run.completion_recorded);
        lua_setfield(lua, run_index, "completion_recorded");
        push_optional_integer(lua, run_index, "target_system_id",
                              run.target_system_id);
        lua_setfield(lua, world_index, "run");
    }

    lua_createtable(lua, 0, 3);
    const int field_index = lua_absindex(lua, -1);
    push_optional_boolean(lua, field_index, "alarm", world.field.alarm);
    push_optional_boolean(lua, field_index, "security_off",
                         world.field.security_off);
    push_optional_boolean(lua, field_index, "maglocks_off",
                         world.field.maglocks_off);
    lua_setfield(lua, world_index, "field");

    if (world.datastore.has_value()) {
        const auto& facts = *world.datastore;
        lua_createtable(lua, 0, 7);
        const int datastore_index = lua_absindex(lua, -1);
        lua_pushinteger(lua, facts.system_id);
        lua_setfield(lua, datastore_index, "system_id");
        push_optional_integer(lua, datastore_index, "tips_page3_flags",
                              facts.tips_page3_flags);
        push_optional_integer(lua, datastore_index, "tips_page2_flags",
                              facts.tips_page2_flags);
        lua_pushinteger(lua, facts.world_event_flags);
        lua_setfield(lua, datastore_index, "world_event_flags");
        push_optional_integer(lua, datastore_index, "group_item_flags",
                              facts.group_item_flags);
        push_optional_integer(lua, datastore_index, "nuyen", facts.nuyen);
        push_optional_integer(lua, datastore_index, "mr_johnsons_met_flags",
                              facts.mr_johnsons_met_flags);
        lua_setfield(lua, world_index, "datastore");
    }

    lua_createtable(lua, static_cast<int>(world.actors.size()), 0);
    const int actors_index = lua_absindex(lua, -1);
    for (std::size_t index = 0; index < world.actors.size(); ++index) {
        const auto& actor = world.actors[index];
        lua_createtable(lua, 0, 2);
        lua_pushlstring(lua, actor.actor_id.data(), actor.actor_id.size());
        lua_setfield(lua, -2, "id");
        push_actor_roles(lua, lua_absindex(lua, -1), actor.roles);
        lua_rawseti(lua, actors_index, static_cast<lua_Integer>(index + 1U));
    }
    lua_setfield(lua, world_index, "actors");
}

ScriptMatrixWorldState read_matrix_world_state(lua_State* lua,
                                                const int world_index) {
    ScriptMatrixWorldState world;
    if (const auto value = read_optional_integer(
            lua, world_index, "karma_counter");
        value.has_value()) {
        world.karma_counter = static_cast<std::int8_t>(
            std::clamp<lua_Integer>(*value, -128, 127));
    }
    if (const auto value = read_optional_integer(
            lua, world_index, "passive_ic_counter");
        value.has_value()) {
        world.passive_ic_counter = static_cast<std::int8_t>(
            std::clamp<lua_Integer>(*value, -128, 127));
    }
    lua_getfield(lua, world_index, "map_knowledge");
    if (lua_istable(lua, -1)) {
        const auto knowledge_index = lua_absindex(lua, -1);
        const auto system_id = read_optional_integer(
            lua, knowledge_index, "system_id");
        if (system_id.has_value() && *system_id >= 0 && *system_id <= 24) {
            ScriptMatrixMapKnowledge knowledge;
            knowledge.system_id = static_cast<std::uint8_t>(*system_id);
            lua_getfield(lua, knowledge_index, "cells");
            if (lua_istable(lua, -1)) {
                const auto rows_index = lua_absindex(lua, -1);
                for (std::size_t y = 0; y < knowledge.rows.size(); ++y) {
                    lua_rawgeti(lua, rows_index, static_cast<lua_Integer>(y));
                    if (lua_istable(lua, -1)) {
                        const auto row_index = lua_absindex(lua, -1);
                        for (std::size_t x = 0;
                             x < knowledge.rows[y].size(); ++x) {
                            lua_rawgeti(lua, row_index,
                                        static_cast<lua_Integer>(x));
                            if (lua_istable(lua, -1)) {
                                const auto cell_index = lua_absindex(lua, -1);
                                auto& cell = knowledge.rows[y][x];
                                const auto level = read_optional_integer(
                                    lua, cell_index, "analysis_level");
                                if (level.has_value() && *level >= 0 &&
                                    *level <= 7) {
                                    cell.analysis_level =
                                        static_cast<std::uint8_t>(*level);
                                }
                                cell.security_color =
                                    matrix_security_color_from_lua(
                                        lua, cell_index);
                                cell.revealed = read_optional_boolean(
                                    lua, cell_index, "revealed")
                                                    .value_or(false);
                            }
                            lua_pop(lua, 1);
                        }
                    }
                    lua_pop(lua, 1);
                }
            }
            lua_pop(lua, 1);
            world.map_knowledge = knowledge;
        }
    }
    lua_pop(lua, 1);
    lua_getfield(lua, world_index, "run");
    if (lua_istable(lua, -1)) {
        const int run_index = lua_absindex(lua, -1);
        const auto status = matrix_run_status_from_lua(lua, run_index);
        const auto file_size =
            read_optional_integer(lua, run_index, "file_size");
        const auto completion_marker_present =
            read_optional_boolean(lua, run_index, "completion_marker_present");
        const auto completion_recorded =
            read_optional_boolean(lua, run_index, "completion_recorded");
        const auto target =
            read_optional_integer(lua, run_index, "target_system_id");
        ScriptMatrixRunObjective objective;
        bool objective_node_valid = false;
        bool objective_valid = false;
        lua_getfield(lua, run_index, "objective");
        if (lua_istable(lua, -1)) {
            const int objective_index = lua_absindex(lua, -1);
            lua_getfield(lua, objective_index, "node");
            if (lua_type(lua, -1) == LUA_TSTRING) {
                std::size_t length = 0U;
                const char* value = lua_tolstring(lua, -1, &length);
                if (value != nullptr) {
                    const std::string_view name(value, length);
                    if (name == "cpu") {
                        objective.node = ScriptMatrixRunNode::cpu;
                        objective_node_valid = true;
                    } else if (name == "datastore") {
                        objective.node = ScriptMatrixRunNode::datastore;
                        objective_node_valid = true;
                    }
                }
            }
            lua_pop(lua, 1);

            lua_getfield(lua, objective_index, "action");
            if (lua_type(lua, -1) == LUA_TSTRING) {
                std::size_t length = 0U;
                const char* value = lua_tolstring(lua, -1, &length);
                if (value != nullptr) {
                    const std::string_view name(value, length);
                    if (name == "crash_system") {
                        objective.action = ScriptMatrixRunAction::crash_system;
                        objective_valid = true;
                    } else if (name == "delete_file") {
                        objective.action = ScriptMatrixRunAction::delete_file;
                        objective_valid = true;
                    } else if (name == "upload_file") {
                        objective.action = ScriptMatrixRunAction::upload_file;
                        objective_valid = true;
                    } else if (name == "download_file") {
                        objective.action = ScriptMatrixRunAction::download_file;
                        objective_valid = true;
                    }
                }
            }
            lua_pop(lua, 1);

            lua_getfield(lua, objective_index, "file_class");
            if (lua_type(lua, -1) == LUA_TSTRING) {
                std::size_t length = 0U;
                const char* value = lua_tolstring(lua, -1, &length);
                if (value != nullptr) {
                    objective.file_class.assign(value, length);
                }
            }
            lua_pop(lua, 1);
            objective.file_id = read_optional_integer(
                lua, objective_index, "file_id");
        }
        lua_pop(lua, 1);
        const bool target_is_valid = !target.has_value() ||
            (*target >= 0 && *target <= 24);
        const bool file_is_valid = !objective.file_id.has_value() ||
            (*objective.file_id >= 0 && *objective.file_id <= 69);
        const bool objective_shape_valid = objective_node_valid &&
            objective_valid && file_is_valid &&
            ((objective.node == ScriptMatrixRunNode::cpu &&
              objective.action == ScriptMatrixRunAction::crash_system &&
              objective.file_class.empty() && !objective.file_id.has_value()) ||
             (objective.node == ScriptMatrixRunNode::datastore &&
              objective.action != ScriptMatrixRunAction::crash_system &&
              !objective.file_class.empty() &&
              ((objective.action == ScriptMatrixRunAction::upload_file &&
                !objective.file_id.has_value()) ||
               ((objective.action == ScriptMatrixRunAction::delete_file ||
                 objective.action == ScriptMatrixRunAction::download_file) &&
                objective.file_id.has_value()))));
        if (status.has_value() && objective_shape_valid &&
            file_size.has_value() && *file_size >= 0 && *file_size <= 255 &&
            completion_marker_present.has_value() &&
            completion_recorded.has_value() &&
            target_is_valid) {
            ScriptMatrixRunState run;
            run.status = *status;
            run.objective = std::move(objective);
            run.file_size = static_cast<int>(*file_size);
            run.completion_marker_present =
                *completion_marker_present;
            run.completion_recorded = *completion_recorded;
            if (target.has_value()) {
                run.target_system_id = static_cast<int>(*target);
            }
            world.run = run;
        }
    }
    lua_pop(lua, 1);

    lua_getfield(lua, world_index, "field");
    if (lua_istable(lua, -1)) {
        const int field_index = lua_absindex(lua, -1);
        world.field.alarm = read_optional_boolean(lua, field_index, "alarm");
        world.field.security_off = read_optional_boolean(
            lua, field_index, "security_off");
        world.field.maglocks_off = read_optional_boolean(
            lua, field_index, "maglocks_off");
    }
    lua_pop(lua, 1);

    lua_getfield(lua, world_index, "datastore");
    if (lua_istable(lua, -1)) {
        const int datastore_index = lua_absindex(lua, -1);
        const auto read_optional_u32 = [lua, datastore_index](const char* name)
            -> std::optional<std::uint32_t> {
            const auto value = read_optional_integer(lua, datastore_index,
                                                     name);
            if (!value.has_value()) {
                return std::nullopt;
            }
            return static_cast<std::uint32_t>(
                std::clamp<lua_Integer>(*value, 0, 0xFFFFFFFFLL));
        };
        ScriptMatrixDatastoreFacts facts;
        facts.system_id = static_cast<std::uint8_t>(
            read_integer(lua, datastore_index, "system_id", 0, 0, 0xFF));
        facts.tips_page3_flags = read_optional_u32("tips_page3_flags");
        facts.tips_page2_flags = read_optional_u32("tips_page2_flags");
        facts.world_event_flags =
            read_optional_u32("world_event_flags").value_or(0U);
        facts.group_item_flags = read_optional_u32("group_item_flags");
        facts.nuyen = read_optional_u32("nuyen");
        if (const auto value = read_optional_integer(
                lua, datastore_index, "mr_johnsons_met_flags");
            value.has_value()) {
            facts.mr_johnsons_met_flags = static_cast<std::uint8_t>(
                std::clamp<lua_Integer>(*value, 0, 0xFFLL));
        }
        world.datastore = facts;
    }
    lua_pop(lua, 1);

    lua_getfield(lua, world_index, "actors");
    if (lua_istable(lua, -1)) {
        const int actors_index = lua_absindex(lua, -1);
        const auto count = lua_rawlen(lua, actors_index);
        world.actors.reserve(count);
        for (std::size_t index = 0; index < count; ++index) {
            lua_rawgeti(lua, actors_index,
                        static_cast<lua_Integer>(index + 1U));
            if (lua_istable(lua, -1)) {
                const int actor_index = lua_absindex(lua, -1);
                lua_getfield(lua, actor_index, "id");
                std::size_t id_size = 0U;
                const char* const id = lua_tolstring(lua, -1, &id_size);
                if (id != nullptr) {
                    const std::string actor_id(id, id_size);
                    lua_pop(lua, 1);
                    const auto roles = read_actor_roles(lua, actor_index);
                    world.actors.push_back(ScriptMatrixFieldActorState{
                        actor_id, roles.value_or(ScriptActorRoleFacts{})});
                } else {
                    lua_pop(lua, 1);
                }
            }
            lua_pop(lua, 1);
        }
    }
    lua_pop(lua, 1);
    return world;
}

std::array<ScriptCyberdeckProgramSlot, 5> read_slots(
    lua_State* lua, const int table_index) {
    std::array<ScriptCyberdeckProgramSlot, 5> slots{};
    for (std::size_t index = 0; index < slots.size(); ++index) {
        lua_rawgeti(lua, table_index, static_cast<lua_Integer>(index));
        if (lua_istable(lua, -1)) {
            const auto slot_index = lua_absindex(lua, -1);
            slots[index].status = static_cast<std::uint8_t>(
                read_integer(lua, slot_index, "status", 0, 0, 0xFF));
            slots[index].timer = static_cast<std::uint16_t>(
                read_integer(lua, slot_index, "timer", 0, 0, 0xFFFF));
            slots[index].phase = static_cast<std::uint8_t>(
                read_integer(lua, slot_index, "phase", 0, 0, 0xFF));
        }
        lua_pop(lua, 1);
    }
    return slots;
}

} // namespace

int dispatch_cyberdeck_menu_binding(
    lua_State* lua, ScriptMenuRouteGame& game,
    const CyberdeckMenuBinding binding) {
    switch (binding) {
    case CyberdeckMenuBinding::enter:
        lua_pushinteger(lua, game.deck_enter ? game.deck_enter() : 0);
        return 1;
    case CyberdeckMenuBinding::leave:
        if (game.deck_leave) game.deck_leave(integer_argument(lua, 1));
        return 0;
    case CyberdeckMenuBinding::set_mode:
        if (game.set_deck_mode) {
            const int source_mode = integer_argument(lua, 1);
            // FFF0B7 is a byte in the ROM: attack is 0xFF, balanced is 0,
            // and defense is 1. The host API uses signed -1/0/1 values.
            const int host_mode = source_mode == 0xFF
                                      ? -1
                                      : std::clamp(source_mode, 0, 1);
            game.set_deck_mode(host_mode);
        }
        return 0;
    case CyberdeckMenuBinding::matrix_online:
        lua_pushboolean(lua, game.matrix_online && game.matrix_online());
        return 1;
    case CyberdeckMenuBinding::matrix_node: {
        const auto node = game.matrix_node ? game.matrix_node() : std::nullopt;
        if (!node.has_value()) {
            lua_pushnil(lua);
            return 1;
        }
        lua_createtable(lua, 0, 4);
        lua_pushinteger(lua, node->ic_condition);
        lua_setfield(lua, -2, "ic_condition");
        lua_pushinteger(lua, node->ic_code);
        lua_setfield(lua, -2, "ic_code");
        lua_pushinteger(lua, node->ic_rating);
        lua_setfield(lua, -2, "ic_rating");
        lua_pushinteger(lua, node->ic_rating2);
        lua_setfield(lua, -2, "ic_rating2");
        return 1;
    }
    case CyberdeckMenuBinding::alert:
        lua_pushinteger(lua, game.alert ? game.alert() : -1);
        return 1;
    case CyberdeckMenuBinding::world_effect_gate_negative:
        lua_pushboolean(lua, game.world_effect_gate_negative &&
                                  game.world_effect_gate_negative());
        return 1;
    case CyberdeckMenuBinding::matrix_node_discovered:
        lua_pushboolean(lua, game.matrix_node_discovered &&
                                  game.matrix_node_discovered());
        return 1;
    case CyberdeckMenuBinding::matrix_world_state:
        push_matrix_world_state(
            lua, game.matrix_world_state ? game.matrix_world_state()
                                         : ScriptMatrixWorldState{});
        return 1;
    case CyberdeckMenuBinding::commit_matrix_world_state:
        if (game.commit_matrix_world_state && lua_istable(lua, 1)) {
            game.commit_matrix_world_state(
                read_matrix_world_state(lua, lua_absindex(lua, 1)));
        }
        return 0;
    case CyberdeckMenuBinding::record:
        push_record(lua, game.deck_record ? game.deck_record()
                                          : ScriptCyberdeckRecord{});
        return 1;
    case CyberdeckMenuBinding::program_slots: {
        const auto slots = game.deck_slots
                               ? game.deck_slots()
                               : std::array<ScriptCyberdeckProgramSlot, 5>{};
        lua_createtable(lua, static_cast<int>(slots.size()), 0);
        const auto slots_index = lua_absindex(lua, -1);
        for (std::size_t index = 0; index < slots.size(); ++index) {
            lua_createtable(lua, 0, 3);
            lua_pushinteger(lua, slots[index].status);
            lua_setfield(lua, -2, "status");
            lua_pushinteger(lua, slots[index].timer);
            lua_setfield(lua, -2, "timer");
            lua_pushinteger(lua, slots[index].phase);
            lua_setfield(lua, -2, "phase");
            lua_rawseti(lua, slots_index, static_cast<lua_Integer>(index));
        }
        return 1;
    }
    case CyberdeckMenuBinding::commit:
        if (game.deck_commit && lua_istable(lua, 1) && lua_istable(lua, 2)) {
            game.deck_commit(read_record(lua, lua_absindex(lua, 1)),
                             read_slots(lua, lua_absindex(lua, 2)));
        }
        return 0;
    case CyberdeckMenuBinding::program_unloaded:
        if (game.program_unloaded) {
            game.program_unloaded(integer_argument(lua, 1));
        }
        return 0;
    }
    lua_pushnil(lua);
    return 1;
}

} // namespace shadowrun::ghidra_conversion::scripting
