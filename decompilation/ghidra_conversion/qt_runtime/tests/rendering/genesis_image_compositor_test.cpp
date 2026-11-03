#include "genesis_image_compositor.hpp"

#include <QColor>
#include <QImage>

#include <array>
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

    // GPGX core/vdp_render.c palette_init() applies MAKE_PIXEL to
    // (CRAM level * 2) for normal colors; its libretro RGB565 callback expands
    // the packed channels by channel * 255 / max. These outputs are measured
    // from the GPGX L36 inventory and UI palette captures [C].
    constexpr std::array<int, 8> kGpgxRedBlue{{
        0, 32, 65, 98, 139, 172, 205, 238,
    }};
    constexpr std::array<int, 8> kGpgxGreen{{
        0, 32, 68, 101, 137, 170, 206, 238,
    }};
    for (std::uint16_t level = 0U; level < 8U; ++level) {
        const auto word = static_cast<std::uint16_t>(
            (level << 1U) | (level << 5U) | (level << 9U));
        if (!expect(genesis_cram_word_to_color(word) ==
                        QColor(kGpgxRedBlue[level], kGpgxGreen[level],
                               kGpgxRedBlue[level]),
                    "CRAM-to-RGB must match GPGX RGB565 channel expansion")) {
            return 1;
        }
    }
    if (!expect(genesis_cram_word_to_color(0x0640U) ==
                    QColor(0, 68, 98),
                "ROM palette 0x0640 must match the GPGX bar blue") ||
        !expect(genesis_cram_word_to_color(0x000EU) ==
                    QColor(238, 0, 0),
                "ROM palette 0x000E must match the GPGX red") ||
        !expect(genesis_cram_word_to_color(0x00CEU) ==
                    QColor(238, 206, 0),
                "ROM palette 0x00CE must match the GPGX gold") ||
        !expect(genesis_cram_word_to_color(0x0226U) ==
                    QColor(98, 32, 32),
                "ROM palette 0x0226 must match the GPGX dark red")) {
        return 1;
    }

    // These four extracted bar colors correspond to the GPGX-measured
    // inventory pixels above; the fifth pixel verifies alpha preservation.
    QImage legacy_asset(5, 1, QImage::Format_ARGB32);
    legacy_asset.setPixelColor(0, 0, QColor(0, 73, 109, 255));
    legacy_asset.setPixelColor(1, 0, QColor(255, 0, 0, 255));
    legacy_asset.setPixelColor(2, 0, QColor(255, 219, 0, 255));
    legacy_asset.setPixelColor(3, 0, QColor(109, 36, 36, 255));
    legacy_asset.setPixelColor(4, 0, QColor(90, 123, 167, 128));
    const auto normalized_asset =
        convert_legacy_expanded_genesis_image_to_gpgx_rgb565(legacy_asset);
    const auto normalized_alpha_pixel = normalized_asset.pixelColor(4, 0);
    if (!expect(normalized_asset.pixelColor(0, 0) == QColor(0, 68, 98),
                "legacy bar blue must match GPGX (0,68,98)") ||
        !expect(normalized_asset.pixelColor(1, 0) == QColor(238, 0, 0),
                "legacy bar red must match GPGX (238,0,0)") ||
        !expect(normalized_asset.pixelColor(2, 0) == QColor(238, 206, 0),
                "legacy bar gold must match GPGX (238,206,0)") ||
        !expect(normalized_asset.pixelColor(3, 0) == QColor(98, 32, 32),
                "legacy bar dark red must match GPGX (98,32,32)") ||
        !expect(normalized_alpha_pixel == QColor(90, 123, 167, 128),
                "image conversion must preserve partial alpha")) {
        return 1;
    }

    QImage palette_flash(3, 1, QImage::Format_ARGB32_Premultiplied);
    palette_flash.fill(Qt::transparent);
    palette_flash.setPixelColor(0, 0, QColor(139, 137, 101, 255));
    palette_flash.setPixelColor(1, 0, QColor(80, 120, 160, 128));
    const auto white_flash =
        recolor_genesis_image_to_cram_word(palette_flash, 0x0EEEU);
    const auto gray_step =
        recolor_genesis_image_to_cram_word(palette_flash, 0x0CCCU);
    if (!expect(white_flash.pixelColor(0, 0) == QColor(238, 238, 238, 255),
                "0xEEE palette flash must render visible pixels white") ||
        !expect(gray_step.pixelColor(0, 0) == QColor(205, 206, 205, 255),
                "0xCCC palette step must render the source 3-bit gray") ||
        !expect(white_flash.pixelColor(1, 0).alpha() == 128,
                "palette flash must preserve partial alpha") ||
        !expect(white_flash.pixelColor(2, 0).alpha() == 0,
                "palette flash must preserve color-zero transparency")) {
        return 1;
    }

    QImage fasa(1, 1, QImage::Format_ARGB32_Premultiplied);
    fasa.setPixelColor(0, 0, QColor(65, 137, 205, 255));
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
                    QColor(32, 32, 32, 255),
                "FASA first palette pass must advance only one CRAM level") ||
        !expect(fasa_second_pass.pixelColor(0, 0) ==
                    QColor(65, 68, 65, 255),
                "FASA fade-in must clamp each channel to its target level") ||
        !expect(fasa_target.pixelColor(0, 0) == QColor(65, 137, 205, 255),
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
                    QColor(32, 101, 172, 255),
                "FASA fade-out must remove one CRAM level per pass") ||
        !expect(fasa_fade_out_complete.pixelColor(0, 0) ==
                    QColor(0, 0, 0, 255),
                "FASA fade-out must reach black") ||
        !expect(matrix_logout_pass_five.pixelColor(0, 0) ==
                    QColor(0, 0, 32, 255),
                "Matrix logout pass five must preserve the remaining CRAM blue level") ||
        !expect(fasa_target.pixelColor(0, 0).alpha() == 255,
                "FASA fade must preserve alpha")) {
        return 1;
    }
    return 0;
}
