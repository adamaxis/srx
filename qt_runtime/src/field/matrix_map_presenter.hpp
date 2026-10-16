#pragma once

#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <QImage>
#include <QPainter>

#include <array>

namespace shadowrun::ghidra_conversion::qt_runtime {

bool paint_matrix_system_map(
    QPainter& painter,
    const QImage& background,
    const QImage& font,
    const std::array<QImage, 4>& tile_atlases,
    const scripting::ScriptMenuRouteSnapshot& state);

bool paint_matrix_node_action_panel(
    QPainter& painter,
    const QImage& font,
    const scripting::ScriptMenuRouteSnapshot& state);

} // namespace shadowrun::ghidra_conversion::qt_runtime
