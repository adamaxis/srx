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
    QImage hud(320, 224, QImage::Format_ARGB32_Premultiplied);
    hud.fill(Qt::black);
    make_field_hud_layer_transparent_outside_chrome(hud);
    if (!expect(hud.pixelColor(100, 100).alpha() == 0,
                "field playfield color zero must be transparent") ||
        !expect(hud.pixelColor(280, 100).alpha() == 255,
                "field six-tile chrome must remain opaque")) {
        return 1;
    }

    QImage faded(1, 1, QImage::Format_ARGB32_Premultiplied);
    faded.setPixelColor(0, 0, QColor(200, 100, 40, 255));
    faded = scale_genesis_image_rgb(faded, 128U);
    if (!expect(faded.pixelColor(0, 0).red() == 100 &&
                    faded.pixelColor(0, 0).green() == 50 &&
                    faded.pixelColor(0, 0).blue() == 20,
                "palette fade must scale RGB without changing alpha") ||
        !expect(faded.pixelColor(0, 0).alpha() == 255,
                "palette fade must preserve sprite alpha")) {
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
    if (!expect(fasa_fade_out_first.pixelColor(0, 0) ==
                    QColor(36, 109, 182, 255),
                "FASA fade-out must remove one CRAM level per pass") ||
        !expect(fasa_fade_out_complete.pixelColor(0, 0) ==
                    QColor(0, 0, 0, 255),
                "FASA fade-out must reach black") ||
        !expect(fasa_target.pixelColor(0, 0).alpha() == 255,
                "FASA fade must preserve alpha")) {
        return 1;
    }
    return 0;
}
