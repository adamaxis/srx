#include "matrix_map_presenter.hpp"

#include "enums.hpp"

#include <QPoint>
#include <QRect>
#include <QSize>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr int kScreenWidth = 320;
constexpr int kScreenHeight = 224;
constexpr int kMapColumns = 14;
constexpr int kCellOriginX = 64;
constexpr int kCellOriginY = 24;
constexpr int kMapLabelOriginY = 32;
constexpr int kMapLabelStrideY = 32;
constexpr std::size_t kCellCount = 140U;
constexpr std::uint16_t kFirstMapPattern = 0x00BFU;
constexpr std::uint16_t kLastMapPattern = 0x00D8U;

void draw_map_labels(QPainter& painter,
                     const QImage& font,
                     const std::array<std::string, 6>& labels) {
    for (std::size_t label = 0U; label < labels.size(); ++label) {
        auto x = 16;
        // ROM 0x532BC starts at plane-A address 0xC204 (x=16, y=32).
        // Each label in 0x15FD36 is followed by four 0x80 controls. In
        // render_encoded_title_text (0xC0DA), each control advances A2 by
        // 0x80 bytes, one 8-pixel tile row, so labels are 32 pixels apart.
        const auto y = kMapLabelOriginY +
                       static_cast<int>(label) * kMapLabelStrideY;
        for (const auto character : labels[label]) {
            const auto code = static_cast<unsigned char>(character);
            if (code < 0x20U || code > 0x7EU) {
                x += 8;
                continue;
            }
            const auto glyph = static_cast<int>(code - 0x20U);
            const auto source_x = (glyph % 16) * 8;
            const auto source_y = (glyph / 16) * 8;
            if (source_x + 8 <= font.width() &&
                source_y + 8 <= font.height()) {
                painter.drawImage(QPoint(x, y), font,
                                  QRect(source_x, source_y, 8, 8));
            }
            x += 8;
        }
    }
}

void draw_map_cells(
    QPainter& painter,
    const std::array<QImage, 4>& tile_atlases,
    const std::array<scripting::ScriptMatrixMapCellVisual, kCellCount>& cells) {
    for (std::size_t cell = 0U; cell < cells.size(); ++cell) {
        const auto column = static_cast<int>(cell % kMapColumns);
        const auto row = static_cast<int>(cell / kMapColumns);
        const auto cell_x = kCellOriginX + column * 16;
        const auto cell_y = kCellOriginY + row * 16;
        for (std::size_t quadrant = 0U; quadrant < 4U; ++quadrant) {
            const auto word = cells[cell].tile_words[quadrant];
            const auto palette = static_cast<std::size_t>(
                (word >> 13U) & 0x03U);
            const auto pattern = static_cast<std::uint16_t>(word & 0x07FFU);
            if (palette == 0U || palette >= tile_atlases.size() ||
                tile_atlases[palette].isNull() ||
                pattern < kFirstMapPattern || pattern > kLastMapPattern) {
                continue;
            }

            const auto tile_index = static_cast<std::size_t>(
                pattern - kFirstMapPattern);
            const QRect source(
                static_cast<int>((tile_index % 16U) * 8U),
                static_cast<int>((tile_index / 16U) * 8U), 8, 8);
            const auto tile_x = cell_x +
                static_cast<int>((quadrant % 2U) * 8U);
            const auto tile_y = cell_y +
                static_cast<int>((quadrant / 2U) * 8U);
            const bool flip_x = (word & 0x0800U) != 0U;
            const bool flip_y = (word & 0x1000U) != 0U;
            painter.save();
            painter.translate(tile_x, tile_y);
            if (flip_x) {
                painter.translate(8, 0);
                painter.scale(-1.0, 1.0);
            }
            if (flip_y) {
                painter.translate(0, 8);
                painter.scale(1.0, -1.0);
            }
            painter.drawImage(QPoint(0, 0), tile_atlases[palette], source);
            painter.restore();
        }
    }
}

} // namespace

bool paint_matrix_system_map(
    QPainter& painter,
    const QImage& background,
    const QImage& font,
    const std::array<QImage, 4>& tile_atlases,
    const scripting::ScriptMenuRouteSnapshot& state) {
    if (background.size() != QSize(kScreenWidth, kScreenHeight) ||
        font.size() != QSize(128, 48) ||
        !state.top_matrix_map_cells_present) {
        return false;
    }
    for (std::size_t palette = 1U; palette < tile_atlases.size(); ++palette) {
        if (tile_atlases[palette].size() != QSize(128, 16)) {
            return false;
        }
    }

    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawImage(QPoint(0, 0), background);
    if (state.top_matrix_map_labels_present) {
        draw_map_labels(painter, font, state.top_matrix_map_labels);
    }
    draw_map_cells(painter, tile_atlases, state.top_matrix_map_cells);
    painter.restore();
    return true;
}

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
    const scripting::ScriptMenuRouteSnapshot& state) {
    if (state.top_screen != static_cast<std::uint16_t>(
                                srx::MenuScreen::MATRIX_NODE_ACTION) ||
        !state.top_matrix_node_status.has_value() ||
        !state.top_matrix_traversal_view.has_value() ||
        hud_frame.size() != QSize(kScreenWidth, kScreenHeight) ||
        runner.size() != QSize(kScreenWidth, kScreenHeight) ||
        font.size() != QSize(128, 48) ||
        empty_program_icon.size() != QSize(24, 16)) {
        return false;
    }

    const auto& view = *state.top_matrix_traversal_view;
    const auto model_id = static_cast<std::size_t>(view.world_model_id);
    const auto palette_id = static_cast<std::size_t>(view.palette_theme);
    if (model_id >= world_models.size() || view.palette_theme < 0 ||
        palette_id >= world_models[model_id].size() ||
        backgrounds[0].size() != QSize(kScreenWidth, kScreenHeight) ||
        (model_id == 0U &&
         world_models[model_id][palette_id].size() != QSize(112, 64)) ||
        (model_id != 0U &&
         world_models[model_id][palette_id].size() != QSize(96, 80))) {
        return false;
    }

    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    // The ROM selects the model palette from the knowledge rank: row 0 for
    // an unknown node, then rows 1..4 for its security color. Applying that
    // palette to the floor would recolor a ROM-blue viewport.
    // The independent traversal frames (for example longplay frame 004440)
    // show the blue grid even when the SAN status reads Green-4.
    painter.drawImage(QPoint(0, 0), backgrounds[0]);
    // The ROM writes these status brackets as two small Plane-A grid batches
    // after the Plane-B floor/footer. Keep the extracted HUD above the floor
    // and below the selected object/effect and runner sprite layers.
    painter.drawImage(QPoint(0, 0), hud_frame);

    // ROM 0x177FC's 61-VBlank HUD setup precedes 0x2679E's selected entry
    // stream. Lua carries those counters and the measured model canvas; this
    // presenter only places the already extracted image. The System 2 SPU
    // projection is calibrated to independent Section 21 frames 004730-004830.
    if (view.world_entry_phase != 0) {
        if (view.model_canvas_width > 0 && view.model_canvas_height > 0) {
            painter.drawImage(
                QRect(view.model_canvas_x, view.model_canvas_y,
                      view.model_canvas_width, view.model_canvas_height),
                world_models[model_id][palette_id]);
        } else if (!(model_id == 2U && view.palette_theme == 2 &&
                     view.world_entry_sequence_duration == 239)) {
            // Other model IDs have no same-state growth capture yet. Keep
            // their existing provisional placement visible after setup.
            const QPoint model_origin = model_id == 0U ? QPoint(104, 48)
                                                       : QPoint(110, 28);
            painter.drawImage(model_origin,
                              world_models[model_id][palette_id]);
        }
    }

    const QImage* world_effect = nullptr;
    if (view.world_effect_mode == -1) {
        if (view.world_effect_phase >= 0 &&
            static_cast<std::size_t>(view.world_effect_phase) <
                world_effect_fallback.size()) {
            world_effect = &world_effect_fallback[
                static_cast<std::size_t>(view.world_effect_phase)];
        }
    } else if (view.world_effect_mode >= 0 &&
               view.world_effect_mode <
                   static_cast<int>(world_effect_modes.size())) {
        const auto mode = static_cast<std::size_t>(view.world_effect_mode);
        const auto phase = static_cast<std::size_t>(view.world_effect_phase);
        if (phase < world_effect_modes[mode].size()) {
            world_effect = &world_effect_modes[mode][phase];
        }
    }
    if (world_effect != nullptr &&
        world_effect->size() == QSize(kScreenWidth, kScreenHeight)) {
        // 0x1AA40 appends this selected sprite list over the current world
        // object; the runner remains a separate, later sprite layer.
        painter.drawImage(QPoint(0, 0), *world_effect);
    }

    // The compact 239-VBlank entry finishes on FFDF8C figure phase four.
    // This extracted 320x224 host layer preserves the four independently
    // selected ROM sprite groups at their source screen coordinates.
    const QImage* runner_layer = &runner;
    if (view.analyze_runner_phase == 2 &&
        analyze_runner_idle.size() == QSize(kScreenWidth, kScreenHeight)) {
        runner_layer = &analyze_runner_idle;
    } else if (view.analyze_runner_phase == 0 &&
        analyze_runner_phase_0.size() == QSize(kScreenWidth, kScreenHeight)) {
        runner_layer = &analyze_runner_phase_0;
    } else if (view.analyze_runner_phase == 1 &&
               analyze_runner_phase_1.size() ==
                   QSize(kScreenWidth, kScreenHeight)) {
        runner_layer = &analyze_runner_phase_1;
    }
    painter.drawImage(QPoint(0, 0), *runner_layer);

    // Analyze's two ring lists are appended after Behavior A's runner lists
    // by the 0x1BEBA/0x1C136 path. Keep them as separately extracted images
    // and select only while Lua owns an active source-timed transition.
    if (view.analyze_effect_phase >= 0 &&
        static_cast<std::size_t>(view.analyze_effect_phase) <
            analyze_rings.size()) {
        const auto& rings = analyze_rings[
            static_cast<std::size_t>(view.analyze_effect_phase)];
        if (rings.size() == QSize(kScreenWidth, kScreenHeight)) {
            painter.drawImage(QPoint(0, 0), rings);
        }
    }

    // The field-mode background contains the source footer and runner
    // portrait. The live deck contributes five installed/empty program cells
    // and the five slot-status strips at the original positions.
    for (std::size_t slot = 0U; slot < 5U; ++slot) {
        const auto program = state.top_matrix_deck_active_programs[slot];
        const auto& icon = state.top_matrix_deck_active_programs_present &&
                                   program >= 0 && program < 12
            ? program_icons[static_cast<std::size_t>(program)]
            : empty_program_icon;
        if (icon.size() == QSize(24, 16)) {
            painter.drawImage(QPoint(80 + static_cast<int>(slot) * 48, 184),
                              icon);
        }
        if (state.top_matrix_deck_program_slot_status_present) {
            const auto status = static_cast<std::size_t>(
                state.top_matrix_deck_program_slot_status[slot]);
            if (status < slot_status_images.size() &&
                slot_status_images[status].size() == QSize(48, 8)) {
                painter.drawImage(
                    QPoint(80 + static_cast<int>(slot) * 48, 208),
                    slot_status_images[status]);
            }
        }
    }

    const auto draw_text = [&painter, &font](const int x, const int y,
                                             const std::string& value) {
        auto current_x = x;
        for (const auto character : value) {
            const auto code = static_cast<unsigned char>(character);
            if (code < 0x20U || code > 0x7EU) {
                current_x += 8;
                continue;
            }
            const auto glyph = static_cast<int>(code - 0x20U);
            painter.drawImage(QPoint(current_x, y), font,
                              QRect((glyph % 16) * 8,
                                    (glyph / 16) * 8, 8, 8));
            current_x += 8;
        }
    };

    // ROM name-table addresses are converted to screen cells: $C086 active
    // program, $C108 its FFF0E8 state, $C0B0 node function, $C0B8 security
    // band, $C0C6 difficulty, $C12E file class, $C1AE/$C22E IC rows, and
    // $C208 system alert. Knowledge gates come from FFFB30's high nibble.
    if (!view.active_program_label.empty()) {
        draw_text(24, 8, view.active_program_label);
        draw_text(32, 16, view.active_program_status_label);
    }
    draw_text(192, 8, view.function_label);
    draw_text(224, 8, view.alert_band_label);
    draw_text(280, 8, view.action_difficulty_label);
    draw_text(184, 16, view.file_class_label);
    draw_text(184, 24, view.ice_label);
    if (!view.ice_rating_label.empty()) {
        draw_text(184 + view.ice_rating_column * 8, 24,
                  view.ice_rating_label);
    }
    draw_text(184, 32, view.second_ice_label);
    if (!view.second_ice_rating_label.empty()) {
        draw_text(184 + view.second_ice_rating_column * 8, 32,
                  view.second_ice_rating_label);
    }
    draw_text(32, 32, view.alert_label);
    painter.restore();
    return true;
}

bool paint_matrix_node_action_panel(
    QPainter& painter,
    const QImage& font,
    const QImage& background,
    const QImage& action_cursor,
    const scripting::ScriptMenuRouteSnapshot& state) {
    if (state.top_screen != static_cast<std::uint16_t>(
                                srx::MenuScreen::MATRIX_NODE_ACTION) ||
        font.size() != QSize(128, 48) ||
        background.size() != QSize(kScreenWidth, kScreenHeight) ||
        (state.top_row.has_value() && *state.top_row >= 0 &&
         static_cast<std::size_t>(*state.top_row) <
             std::min(state.top_option_labels.size(), std::size_t{5}) &&
         action_cursor.size() != QSize(8, 8))) {
        return false;
    }

    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.drawImage(QPoint(0, 0), background);
    if (paint_matrix_logout_transition_art(painter, state)) {
        painter.restore();
        return true;
    }
    const auto draw_text = [&painter, &font](const int x, const int y,
                                             const std::string& value) {
        auto current_x = x;
        for (const auto character : value) {
            const auto code = static_cast<unsigned char>(character);
            if (code < 0x20U || code > 0x7EU) {
                current_x += 8;
                continue;
            }
            const auto glyph = static_cast<int>(code - 0x20U);
            painter.drawImage(QPoint(current_x, y), font,
                              QRect((glyph % 16) * 8,
                                    (glyph / 16) * 8, 8, 8));
            current_x += 8;
        }
    };
    // BMP assets preserve the tile's RGB values but discard its palette-zero
    // alpha. The ROM tile's color index 0 is transparent over the underlying
    // Matrix plane, so restore that transparency before drawing the 8x8 tile.
    auto cursor_overlay = action_cursor.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < cursor_overlay.height(); ++y) {
        auto* line = reinterpret_cast<QRgb*>(cursor_overlay.scanLine(y));
        for (int x = 0; x < cursor_overlay.width(); ++x) {
            if (qRed(line[x]) == 0 && qGreen(line[x]) == 0 &&
                qBlue(line[x]) == 0) {
                line[x] = qRgba(0, 0, 0, 0);
            }
        }
    }

    if (state.top_matrix_countdown.has_value()) {
        // ROM 0x54B84 prints the 0x166A44 stream at VRAM $C518. Its 0x80
        // breaks advance one 64-cell name-table row each; C216 writes the
        // countdown digit at $C722, the gap between the two hyphens.
        std::size_t start = 0U;
        int y = 80;
        while (start <= state.top_text.size() && y < 144) {
            const auto end = state.top_text.find('|', start);
            const auto value = state.top_text.substr(
                start, end == std::string::npos ? std::string::npos
                                                : end - start);
            draw_text(96, y, value);
            y += 8;
            if (end == std::string::npos) break;
            start = end + 1U;
        }
        draw_text(136, 112, std::to_string(*state.top_matrix_countdown));
        painter.restore();
        return true;
    }

    if (state.top_matrix_node_status.has_value()) {
        // ROM 0x54802 writes the node-function title at $C418. The status
        // fields are separate writes from 0x54A28: file class at $C39A,
        // alert-band label at $C51E, and action difficulty at $C52C. On a
        // 64-cell plane each tile row is 0x80 bytes (8 screen pixels).
        const auto& status = *state.top_matrix_node_status;
        draw_text(96, 64, state.top_title);
        draw_text(104, 56, status.file_class_label);
        draw_text(120, 80, status.alert_band_label);
        draw_text(176, 80, std::to_string(status.action_difficulty));
    } else {
        draw_text(24, 20, state.top_title);
    }

    const bool message_view = state.top_title == "Matrix Message";
    if (!state.top_matrix_node_status.has_value()) {
        std::size_t start = 0U;
        int line = 0;
        while (start <= state.top_text.size() &&
               line < (message_view ? 12 : 3)) {
            const auto end = state.top_text.find('|', start);
            const auto value = state.top_text.substr(
                start, end == std::string::npos ? std::string::npos
                                                : end - start);
            draw_text(24, 50 + line * (message_view ? 12 : 14), value);
            ++line;
            if (end == std::string::npos) break;
            start = end + 1U;
        }
    }

    const auto option_origin_y =
        state.top_matrix_node_status.has_value() ? 104 : 94;
    for (std::size_t row = 0U;
         row < state.top_option_labels.size() && row < 5U; ++row) {
        const auto y = option_origin_y + static_cast<int>(row) * 16;
        if (state.top_row == static_cast<int>(row)) {
            painter.drawImage(QPoint(96, y), cursor_overlay);
        }
        draw_text(state.top_matrix_node_status.has_value() ? 120 : 24,
                  y, state.top_option_labels[row]);
    }
    painter.restore();
    return true;
}

bool paint_matrix_logout_transition_art(
    QPainter& painter,
    const scripting::ScriptMenuRouteSnapshot& state) {
    if (!state.top_matrix_logout_visual.has_value()) return false;
    const auto& visual = *state.top_matrix_logout_visual;
    using Phase = scripting::ScriptMatrixLogoutVisualPhase;
    if (visual.phase == Phase::FADE) return false;

    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.fillRect(painter.viewport(), Qt::black);
    static constexpr std::array<std::uint8_t, 8> kDotRows{{
        0x00U, 0x00U, 0x14U, 0x08U, 0x14U, 0x00U, 0x00U, 0x00U}};
    static constexpr std::array<std::uint8_t, 8> kSparkleRows{{
        0x08U, 0x2AU, 0x1CU, 0xFFU, 0x1CU, 0x2AU, 0x08U, 0x08U}};
    const auto draw_center_tile = [&painter](
                                      const std::array<std::uint8_t, 8>& rows) {
        for (int y = 0; y < static_cast<int>(rows.size()); ++y) {
            for (int x = 0; x < 8; ++x) {
                if ((rows[static_cast<std::size_t>(y)] &
                     (1U << static_cast<unsigned int>(x))) != 0U) {
                    painter.fillRect(QRect(160 + x, 112 + y, 1, 1),
                                     Qt::white);
                }
            }
        }
    };
    if (visual.phase == Phase::CROSS) {
        // The source tiles 0x4B3 and 0x4B4 are repeated through the viewport:
        // 0x4B3 paints row 3 of each 8x8 tile, 0x4B4 paints column 3.
        if (visual.horizontal_right > visual.horizontal_left) {
            painter.fillRect(
                QRect(visual.horizontal_left, 115,
                      visual.horizontal_right - visual.horizontal_left, 1),
                Qt::white);
        }
        if (visual.vertical_bottom > visual.vertical_top) {
            painter.fillRect(
                QRect(163, visual.vertical_top, 1,
                      visual.vertical_bottom - visual.vertical_top),
                Qt::white);
        }
        draw_center_tile(kSparkleRows);
    } else if (visual.phase == Phase::DOT ||
               visual.phase == Phase::SPARKLE) {
        // Exact color-7 pixel rows decoded from the five ROM patterns uploaded
        // at 0x15BE02. Pattern 0x4B2 is the five-pixel dot; 0x4B1 is the
        // eight-pixel sparkle. Both are drawn at the source tile's screen
        // origin (160,112), so their center aligns with the collapsing lines.
        draw_center_tile(visual.phase == Phase::DOT ? kDotRows
                                                     : kSparkleRows);
    }
    painter.restore();
    return true;
}

bool paint_matrix_run_info_text(
    QPainter& painter,
    const QImage& font,
    const scripting::ScriptMenuRouteSnapshot& state) {
    if (font.size() != QSize(128, 48) ||
        !state.top_matrix_run_info.has_value()) {
        return false;
    }

    const auto& info = *state.top_matrix_run_info;
    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    const auto draw_text = [&painter, &font](const int x, const int y,
                                             const std::string& value) {
        auto current_x = x;
        for (const auto character : value) {
            const auto code = static_cast<unsigned char>(character);
            if (code < 0x20U || code > 0x7EU) {
                current_x += 8;
                continue;
            }
            const auto glyph = static_cast<int>(code - 0x20U);
            painter.drawImage(QPoint(current_x, y), font,
                              QRect((glyph % 16) * 8,
                                    (glyph / 16) * 8, 8, 8));
            current_x += 8;
        }
    };
    const auto draw_text_rows = [&draw_text](const int x, const int y,
                                              const std::string& value) {
        std::size_t start = 0U;
        int row = 0;
        while (start <= value.size()) {
            const auto end = value.find('|', start);
            draw_text(x, y + row * 8,
                      value.substr(start, end == std::string::npos
                                              ? std::string::npos
                                              : end - start));
            ++row;
            if (end == std::string::npos) break;
            start = end + 1U;
        }
    };

    switch (info.kind) {
    case scripting::ScriptMatrixRunInfoKind::no_run:
        // ROM 0x18C50 writes at $C42C. Its 0x80 byte is a row control, so
        // the full message occupies y=64 and y=72 rather than running past
        // the visible 320-pixel plane edge.
        draw_text_rows(176, 64, info.message);
        break;
    case scripting::ScriptMatrixRunInfoKind::complete:
        // ROM 0x18C66 uses the same 0x80 row control, starting at $C42E.
        draw_text_rows(184, 64, info.message);
        break;
    case scripting::ScriptMatrixRunInfoKind::active:
        // ROM 0x18C76 writes the three labels at $C328. The values are
        // separate writes at $C334, $C4AC, and $C630. A 64-cell name-table
        // row is 0x80 bytes, so each address maps directly to these 8x8
        // pixel coordinates.
        draw_text(160, 48, info.node_label);
        draw_text(208, 48, info.node);
        draw_text(160, 64, info.function_label);
        draw_text(176, 72, info.node_function);
        draw_text(160, 88, info.action_label);
        draw_text(192, 96, info.action);
        break;
    }

    painter.restore();
    return true;
}

bool paint_matrix_deck_stats_text(
    QPainter& painter,
    const QImage& font,
    const scripting::ScriptMenuRouteSnapshot& state) {
    if (font.size() != QSize(128, 48) ||
        !state.top_matrix_deck_stats.has_value()) {
        return false;
    }

    const auto& stats = *state.top_matrix_deck_stats;
    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    const auto draw_text = [&painter, &font](const int x, const int y,
                                             const std::string& value) {
        auto current_x = x;
        for (const auto character : value) {
            const auto code = static_cast<unsigned char>(character);
            if (code < 0x20U || code > 0x7EU) {
                current_x += 8;
                continue;
            }
            const auto glyph = static_cast<int>(code - 0x20U);
            painter.drawImage(QPoint(current_x, y), font,
                              QRect((glyph % 16) * 8,
                                    (glyph / 16) * 8, 8, 8));
            current_x += 8;
        }
    };
    const auto grouped_decimal = [](const unsigned int number) {
        auto value = std::to_string(number);
        auto position = value.size();
        while (position > 3U) {
            position -= 3U;
            value.insert(position, 1U, ',');
        }
        return value;
    };

    // ROM 0x18BA2 writes maker/model rows at $C1AE. Each source 0x80
    // separator advances one name-table row, or eight pixels.
    draw_text(184, 24, stats.manufacturer);
    draw_text(184, 32, stats.model);

    // The ROM text stream at 0xE3902 starts with a 12-glyph header rule and
    // then writes the complete dotted labels. Decimal glyphs replace the
    // final dot cells from the right edge of each value field.
    draw_text(160, 40, std::string("  [") + std::string(12U, '\\'));
    constexpr std::array<int, 10> label_y{
        48, 56, 72, 80, 88, 104, 120, 128, 136, 144};
    for (std::size_t index = 0; index < stats.labels.size(); ++index) {
        draw_text(160, label_y[index], stats.labels[index]);
        const auto value = grouped_decimal(stats.values[index]);
        draw_text(280 - static_cast<int>(value.size() - 1U) * 8,
                  label_y[index], value);
    }

    painter.restore();
    return true;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
