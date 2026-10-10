#include "numeric_visual_renderer.hpp"

#include <QPainter>

using namespace shadowrun::ghidra_conversion::qt_runtime;

int main() {
    QImage atlas(128, 48, QImage::Format_ARGB32_Premultiplied);
    atlas.fill(Qt::transparent);
    for (const auto code : {'0', '1', '2', '3', '4', ','}) {
        const auto glyph = static_cast<int>(code - 0x20);
        const auto x = (glyph % 16) * 8;
        const auto y = (glyph / 16) * 8;
        atlas.setPixelColor(x + 2, y + 3, QColor(255, 255, 255, 255));
    }

    const auto grouped = render_numeric_visual(atlas, 1234U, true);
    if (grouped.size() != QSize(40, 8) ||
        grouped.pixelColor(2, 3).alpha() == 0 ||
        grouped.pixelColor(10, 3).alpha() == 0 ||
        grouped.pixelColor(18, 3).alpha() == 0 ||
        grouped.pixelColor(26, 3).alpha() == 0 ||
        grouped.pixelColor(34, 3).alpha() == 0 ||
        grouped.pixelColor(0, 0).alpha() != 0) {
        return 1;
    }

    const auto plain = render_numeric_visual(
        atlas, 42U, false, QColor(0, 255, 0));
    const auto green = plain.pixelColor(2, 3);
    if (plain.size() != QSize(16, 8) || green.red() != 0 ||
        green.green() != 255 || green.blue() != 0 || green.alpha() != 255) {
        return 2;
    }

    QImage canvas(96, 16, QImage::Format_ARGB32_Premultiplied);
    canvas.fill(Qt::transparent);
    QPainter painter(&canvas);
    draw_numeric_visual(painter, atlas, 80, 4, 1234U,
                        NumericVisualAlignment::right, true);
    painter.end();
    if (canvas.pixelColor(40 + 2, 4 + 3).alpha() == 0 ||
        canvas.pixelColor(80 + 2, 4 + 3).alpha() != 0) {
        return 3;
    }

    return 0;
}
