#pragma once

#include "shadowrun_ghidra/scripting/game_script_runtime.hpp"

#include <QImage>
#include <QPainter>

#include <array>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

bool paint_matrix_system_map(
    QPainter& painter,
    const QImage& background,
    const QImage& font,
    const std::array<QImage, 4>& tile_atlases,
    const scripting::ScriptMenuRouteSnapshot& state);

bool paint_matrix_traversal_view(
    QPainter& painter,
    const std::array<QImage, 4>& backgrounds,
    const std::array<std::array<QImage, 5>, 6>& world_models,
    const std::array<std::vector<QImage>, 9>& world_effect_modes,
    const std::vector<QImage>& world_effect_fallback,
    const QImage& hud_frame,
    const QImage& runner,
    const QImage& analyze_runner_phase_0,
    const QImage& analyze_runner_phase_1,
    const QImage& analyze_runner_idle,
    const std::array<QImage, 4>& analyze_rings,
    const QImage& font,
    const std::array<QImage, 16>& program_icons,
    const QImage& empty_program_icon,
    const std::array<QImage, 7>& slot_status_images,
    const scripting::ScriptMenuRouteSnapshot& state);

bool paint_matrix_logout_transition_art(
    QPainter& painter,
    const scripting::ScriptMenuRouteSnapshot& state);

bool paint_matrix_node_action_panel(
    QPainter& painter,
    const QImage& font,
    const QImage& background,
    const QImage& action_cursor,
    const scripting::ScriptMenuRouteSnapshot& state);

bool paint_matrix_run_info_text(
    QPainter& painter,
    const QImage& font,
    const scripting::ScriptMenuRouteSnapshot& state);

bool paint_matrix_deck_stats_text(
    QPainter& painter,
    const QImage& font,
    const scripting::ScriptMenuRouteSnapshot& state);

} // namespace shadowrun::ghidra_conversion::qt_runtime
