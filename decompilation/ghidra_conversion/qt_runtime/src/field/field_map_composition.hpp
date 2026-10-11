#pragma once

#include <QImage>
#include <QRect>

#include <functional>

class QPainter;

namespace shadowrun::ghidra_conversion::qt_runtime {

// Compose a map viewport with low-priority field sprites between the map's
// flattened color and its sparse ROM high-priority pixels.
void paint_field_map_composition(
    QPainter& painter, const QImage& map, const QImage& priority_overlay,
    const QRect& source_rect,
    const std::function<void(QPainter&)>& paint_low_priority_sprites);

} // namespace shadowrun::ghidra_conversion::qt_runtime
