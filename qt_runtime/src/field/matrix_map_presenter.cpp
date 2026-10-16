#include "matrix_map_presenter.hpp"

#include "enums.hpp"

#include <QPoint>
#include <QRect>
#include <QSize>

#include <cstddef>
#include <cstdint>

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

bool paint_matrix_node_action_panel(
    QPainter& painter,
    const QImage& font,
    const scripting::ScriptMenuRouteSnapshot& state) {
    if (state.top_screen != static_cast<std::uint16_t>(
                                srx::MenuScreen::MATRIX_NODE_ACTION) ||
        font.size() != QSize(128, 48)) {
        return false;
    }

    // This is the debug route's clean panel presentation. ROM-derived node
    // state and choices arrive from Lua; it deliberately clears the prior
    // screen before painting so Pocket Secretary controls cannot show through.
    const QColor background(3, 12, 22);
    const QColor panel(7, 27, 39);
    const QColor edge(28, 117, 137);
    const QColor selector(196, 190, 39);
    painter.save();
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    painter.fillRect(QRect(0, 0, 320, 224), background);
    painter.fillRect(QRect(12, 12, 296, 200), panel);
    painter.setPen(edge);
    painter.drawRect(QRect(12, 12, 295, 199));
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
    draw_text(24, 20, state.top_title);
    painter.fillRect(QRect(20, 40, 280, 1), edge);

    const bool message_view = state.top_title == "Matrix Message";
    std::size_t start = 0U;
    int line = 0;
    while (start <= state.top_text.size() && line < (message_view ? 12 : 3)) {
        const auto end = state.top_text.find('|', start);
        const auto value = state.top_text.substr(
            start, end == std::string::npos ? std::string::npos
                                            : end - start);
        draw_text(24, 50 + line * (message_view ? 12 : 14), value);
        ++line;
        if (end == std::string::npos) break;
        start = end + 1U;
    }

    for (std::size_t row = 0U;
         row < state.top_option_labels.size() && row < 5U; ++row) {
        const auto y = 94 + static_cast<int>(row) * 22;
        if (state.top_row == static_cast<int>(row)) {
            painter.fillRect(QRect(20, y - 13, 280, 18),
                             QColor(12, 43, 51));
            painter.fillRect(QRect(20, y - 13, 2, 18), selector);
        }
        draw_text(28, y - 8, state.top_option_labels[row]);
    }
    painter.restore();
    return true;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
