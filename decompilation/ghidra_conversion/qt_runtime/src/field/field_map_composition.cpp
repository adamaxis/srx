#include "field_map_composition.hpp"

#include <QPainter>

namespace shadowrun::ghidra_conversion::qt_runtime {

void paint_field_map_composition(
    QPainter& painter, const QImage& map, const QImage& priority_overlay,
    const QRect& source_rect,
    const std::function<void(QPainter&)>& paint_low_priority_sprites) {
    if (map.isNull() || source_rect.isEmpty()) {
        return;
    }

    painter.drawImage(0, 0, map, source_rect.x(), source_rect.y(),
                      source_rect.width(), source_rect.height());
    if (paint_low_priority_sprites) {
        paint_low_priority_sprites(painter);
    }
    if (!priority_overlay.isNull() &&
        priority_overlay.size() == map.size()) {
        painter.drawImage(0, 0, priority_overlay, source_rect.x(),
                          source_rect.y(), source_rect.width(),
                          source_rect.height());
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
