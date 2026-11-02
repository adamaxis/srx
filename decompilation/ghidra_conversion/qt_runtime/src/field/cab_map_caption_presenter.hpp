#pragma once

#include "shadowrun_ghidra/world/post_intro_mode_conversion.hpp"

#include <QPainter>
#include <QRect>

namespace shadowrun::ghidra_conversion {

inline void paint_cab_map_caption_backing(
    QPainter& painter, const CabModeSelectionPlan& plan) {
    // 0x134D2 composes the Plane-A overlay stream at 0xD4298 over the
    // Plane-B map at 0xD2A2E. 0x13726 writes the padded destination name at
    // 0xEA10; the 18-cell line is black behind white text in longplay frame
    // min_001/f_1650.
    painter.fillRect(
        QRect(plan.caption_x, plan.caption_y,
              static_cast<int>(plan.caption.size()) * 8, 8),
        Qt::black);
}

} // namespace shadowrun::ghidra_conversion
