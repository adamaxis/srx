#pragma once

#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

struct lua_State;

namespace shadowrun::ghidra_conversion::scripting {

enum class CyberdeckMenuBinding {
    enter,
    leave,
    tick,
    set_mode,
    matrix_online,
    matrix_node,
    alert,
    in_alert,
    map_seen,
    matrix_world_state,
    commit_matrix_world_state,
    record,
    slots,
    commit,
    program_unloaded,
};

int dispatch_cyberdeck_menu_binding(
    lua_State* lua, ScriptMenuRouteGame& game,
    CyberdeckMenuBinding binding);

} // namespace shadowrun::ghidra_conversion::scripting
