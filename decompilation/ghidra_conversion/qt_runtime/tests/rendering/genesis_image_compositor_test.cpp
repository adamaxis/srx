#include "genesis_image_compositor.hpp"

#include <QColor>
#include <QImage>

#include <iostream>

namespace {

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion::qt_runtime;

    // Extracted sprites carry their baked color-zero alpha through the V4
    // BMP alpha mask, so the runtime must keep opaque black pixels. The
    // black-wipe helper and the alias transparency gate are gone; this test
    // only covers the surviving compositor utilities.
    // ROM 0x5B2C2/0x5B2E2 keep the intro effect in 15 CRAM words per line;
    // a scalar RGB-weight expectation would bless the removed approximation.
    QImage hud(320, 224, QImage::Format_ARGB32_Premultiplied);
    hud.fill(Qt::black);
    make_field_hud_layer_transparent_outside_chrome(hud);
    if (!expect(hud.pixelColor(100, 100).alpha() == 0,
                "field playfield color zero must be transparent") ||
        !expect(hud.pixelColor(280, 100).alpha() == 255,
                "field six-tile chrome must remain opaque")) {
        return 1;
    }

    QImage palette_flash(3, 1, QImage::Format_ARGB32_Premultiplied);
    palette_flash.fill(Qt::transparent);
    palette_flash.setPixelColor(0, 0, QColor(182, 146, 109, 255));
    palette_flash.setPixelColor(1, 0, QColor(80, 120, 160, 128));
    const auto white_flash =
        recolor_genesis_image_to_cram_word(palette_flash, 0x0EEEU);
    const auto gray_step =
        recolor_genesis_image_to_cram_word(palette_flash, 0x0CCCU);
    if (!expect(white_flash.pixelColor(0, 0) == QColor(255, 255, 255, 255),
                "0xEEE palette flash must render visible pixels white") ||
        !expect(gray_step.pixelColor(0, 0) == QColor(219, 219, 219, 255),
                "0xCCC palette step must render the source 3-bit gray") ||
        !expect(white_flash.pixelColor(1, 0) == QColor(255, 255, 255, 128),
                "palette flash must preserve partial alpha") ||
        !expect(white_flash.pixelColor(2, 0).alpha() == 0,
                "palette flash must preserve color-zero transparency")) {
        return 1;
    }

    QImage fasa(1, 1, QImage::Format_ARGB32_Premultiplied);
    fasa.setPixelColor(0, 0, QColor(73, 146, 219, 255));
    const auto fasa_black =
        transition_genesis_cram_image(fasa, 0U, false);
    const auto fasa_first_pass =
        transition_genesis_cram_image(fasa, 1U, false);
    const auto fasa_second_pass =
        transition_genesis_cram_image(fasa, 2U, false);
    const auto fasa_target =
        transition_genesis_cram_image(fasa, 17U, false);
    if (!expect(fasa_black.pixelColor(0, 0) == QColor(0, 0, 0, 255),
                "FASA fade-in must start from black") ||
        !expect(fasa_first_pass.pixelColor(0, 0) ==
                    QColor(36, 36, 36, 255),
                "FASA first palette pass must advance only one CRAM level") ||
        !expect(fasa_second_pass.pixelColor(0, 0) ==
                    QColor(73, 73, 73, 255),
                "FASA fade-in must clamp each channel to its target level") ||
        !expect(fasa_target.pixelColor(0, 0) == QColor(73, 146, 219, 255),
                "FASA fade-in must reach and preserve the target palette")) {
        return 1;
    }

    const auto fasa_fade_out_first =
        transition_genesis_cram_image(fasa, 1U, true);
    const auto fasa_fade_out_complete =
        transition_genesis_cram_image(fasa, 17U, true);
    // ROM 0x18776 calls 0x1088 exactly five times before its display wipe.
    const auto matrix_logout_pass_five =
        transition_genesis_cram_image(fasa, 5U, true);
    if (!expect(fasa_fade_out_first.pixelColor(0, 0) ==
                    QColor(36, 109, 182, 255),
                "FASA fade-out must remove one CRAM level per pass") ||
        !expect(fasa_fade_out_complete.pixelColor(0, 0) ==
                    QColor(0, 0, 0, 255),
                "FASA fade-out must reach black") ||
        !expect(matrix_logout_pass_five.pixelColor(0, 0) ==
                    QColor(0, 0, 36, 255),
                "Matrix logout pass five must preserve the remaining CRAM blue level") ||
        !expect(fasa_target.pixelColor(0, 0).alpha() == 255,
                "FASA fade must preserve alpha")) {
        return 1;
    }
    return 0;
}
