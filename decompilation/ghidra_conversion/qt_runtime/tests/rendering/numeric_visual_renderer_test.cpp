#include "numeric_visual_renderer.hpp"
#include "shop_balance_display_renderer.hpp"

#include "shadowrun_ghidra/rendering/decimal_glyph_conversion.hpp"

#include <QPainter>

#include <cstdint>

using namespace shadowrun::ghidra_conversion::qt_runtime;

int main() {
    QImage atlas(128, 48, QImage::Format_ARGB32_Premultiplied);
    atlas.fill(Qt::transparent);
    for (const auto code : {
             '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ','}) {
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

    const auto balance_250000 =
        shadowrun::ghidra_conversion::build_numeric_visual(250000U, true);
    // ROM 0xC406/0xC216: digits end in the $CCAC cell (x=176..183); the
    // shop slot spans x=96..183, so 999,999,999 fills it exactly. Larger
    // ten-digit values extend left past the slot as on hardware. Values whose
    // 0xC216 DIVU.W fallback overflows (e.g. 0xFFFFFFFF) are not asserted.
    const auto balance_slot_max =
        shadowrun::ghidra_conversion::build_numeric_visual(999999999U, true);
    const auto balance_ten_digit =
        shadowrun::ghidra_conversion::build_numeric_visual(0x7FFCFFFFU, true);
    if (balance_250000.bitmap_font_text() != "250,000" ||
        balance_250000.pixel_width() != 56U ||
        balance_slot_max.bitmap_font_text() != "999,999,999" ||
        balance_slot_max.pixel_width() != 88U ||
        balance_ten_digit.bitmap_font_text() != "2,147,287,039" ||
        balance_ten_digit.pixel_width() != 104U) {
        return 4;
    }

    const auto shop_balance_image = [&](const std::uint32_t balance) {
        QImage image(256, 224, QImage::Format_ARGB32_Premultiplied);
        image.fill(Qt::transparent);
        QPainter balance_painter(&image);
        draw_field_shop_balance(balance_painter, atlas, balance);
        balance_painter.end();
        return image;
    };
    const auto image_250000 = shop_balance_image(250000U);
    if (image_250000.pixelColor(130, 203).alpha() == 0 ||
        image_250000.pixelColor(178, 203).alpha() == 0 ||
        image_250000.pixelColor(186, 203).alpha() != 0) {
        return 5;
    }
    const auto image_slot_max = shop_balance_image(999999999U);
    if (image_slot_max.pixelColor(94, 203).alpha() != 0 ||
        image_slot_max.pixelColor(98, 203).alpha() == 0 ||
        image_slot_max.pixelColor(178, 203).alpha() == 0 ||
        image_slot_max.pixelColor(186, 203).alpha() != 0) {
        return 6;
    }
    const auto image_ten_digit = shop_balance_image(0x7FFCFFFFU);
    if (image_ten_digit.pixelColor(82, 203).alpha() == 0 ||
        image_ten_digit.pixelColor(178, 203).alpha() == 0 ||
        image_ten_digit.pixelColor(186, 203).alpha() != 0) {
        return 7;
    }

    return 0;
}
