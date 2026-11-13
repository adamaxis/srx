#pragma once

#include <QColor>
#include <QImage>
#include <QPainter>

#include <cstdint>
#include <optional>

namespace shadowrun::ghidra_conversion::qt_runtime {

void draw_field_shop_balance(QPainter& painter, const QImage& font,
                             std::uint32_t balance,
                             std::optional<QColor> tint = std::nullopt);

} // namespace shadowrun::ghidra_conversion::qt_runtime
