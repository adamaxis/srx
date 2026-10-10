#pragma once

#include <QColor>
#include <QImage>
#include <QPainter>

#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion::qt_runtime {

enum class NumericVisualAlignment {
    left,
    right,
};

// Convert a value to a transparent run of 8x8 glyphs clipped from the
// selected extracted font atlas. Menus may choose their own atlas and tint,
// while sharing number formatting, grouping, spacing, and right alignment.
QImage render_numeric_visual(
    const QImage& font_atlas,
    std::uint32_t value,
    bool group_thousands = false,
    std::optional<QColor> tint = std::nullopt);

void draw_numeric_visual(
    QPainter& painter,
    const QImage& font_atlas,
    int x,
    int y,
    std::uint32_t value,
    NumericVisualAlignment alignment = NumericVisualAlignment::left,
    bool group_thousands = false,
    std::optional<QColor> tint = std::nullopt);

} // namespace shadowrun::ghidra_conversion::qt_runtime
