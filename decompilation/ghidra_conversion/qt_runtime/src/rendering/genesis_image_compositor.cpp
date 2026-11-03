#include "genesis_image_compositor.hpp"

#include <QColor>

#include <algorithm>
#include <array>
#include <string_view>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr std::array<int, 8> kLegacyExpandedLevels{{
    0, 36, 73, 109, 146, 182, 219, 255,
}};

int gpgx_rgb565_component(const int level, const bool green) noexcept {
    // Genesis Plus GX palette_init() stores the normal CRAM levels as
    // four-bit channel values (level * 2); MAKE_PIXEL then expands them into
    // RGB565. The libretro RGB565 callback expands those packed channels with
    // channel * 255 / max. See GPGX core/vdp_render.c and retro.py.
    const auto four_bit = level * 2;
    if (green) {
        const auto six_bit = (four_bit << 2) | (four_bit >> 2);
        return six_bit * 255 / 63;
    }
    const auto five_bit = (four_bit << 1) | (four_bit >> 3);
    return five_bit * 255 / 31;
}

int legacy_level_for_component(const int component) noexcept {
    const auto found = std::find(kLegacyExpandedLevels.begin(),
                                 kLegacyExpandedLevels.end(), component);
    return found == kLegacyExpandedLevels.end()
        ? -1
        : static_cast<int>(found - kLegacyExpandedLevels.begin());
}

int gpgx_level_for_component(const int component,
                             const bool green) noexcept {
    for (int level = 0; level < 8; ++level) {
        if (gpgx_rgb565_component(level, green) == component) {
            return level;
        }
    }
    return legacy_level_for_component(component);
}

bool is_black(const QColor& pixel) noexcept {
    return pixel.red() == 0 && pixel.green() == 0 &&
           pixel.blue() == 0;
}

void clear_black_pixel(QImage& image, const int x, const int y) {
    auto pixel = image.pixelColor(x, y);
    if (is_black(pixel)) {
        pixel.setAlpha(0);
        image.setPixelColor(x, y, pixel);
    }
}

} // namespace

QColor genesis_cram_word_to_color(const std::uint16_t color_word) {
    // Genesis CRAM words are RRR0 GGG0 BBB0: red is bits 1..3, green 5..7,
    // and blue 9..11. The field palette upload at 0x1406 and the shared
    // 48-word copy at 0x566C2 establish these values as the field CRAM source
    // [C: VDP CRAM format; ROM 0x1406/0x566C2].
    const auto red = static_cast<int>((color_word >> 1U) & 7U);
    const auto green = static_cast<int>((color_word >> 5U) & 7U);
    const auto blue = static_cast<int>((color_word >> 9U) & 7U);
    return QColor(
        gpgx_rgb565_component(red, false),
        gpgx_rgb565_component(green, true),
        gpgx_rgb565_component(blue, false));
}

QImage convert_legacy_expanded_genesis_image_to_gpgx_rgb565(
    const QImage& image) {
    if (image.isNull()) {
        return image;
    }

    // Keep straight-alpha components while remapping. Converting a partial
    // alpha texel through premultiplied storage can round its RGB values even
    // when the Genesis level is unchanged.
    auto converted = image.convertToFormat(QImage::Format_ARGB32);
    for (int y = 0; y < converted.height(); ++y) {
        for (int x = 0; x < converted.width(); ++x) {
            auto pixel = converted.pixelColor(x, y);
            if (pixel.alpha() == 0) {
                continue;
            }
            const auto remap = [](const int component, const bool green) {
                const auto level = legacy_level_for_component(component);
                return level < 0 ? component
                                 : gpgx_rgb565_component(level, green);
            };
            pixel.setRgb(remap(pixel.red(), false),
                         remap(pixel.green(), true),
                         remap(pixel.blue(), false), pixel.alpha());
            converted.setPixelColor(x, y, pixel);
        }
    }
    return converted;
}

QImage recolor_genesis_image_to_cram_word(const QImage& image,
                                          const std::uint16_t color_word) {
    if (image.isNull()) {
        return image;
    }

    // Palette recoloring uses the same RGB565 channel mapping as the display.
    const auto color = genesis_cram_word_to_color(color_word);
    auto recolored = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < recolored.height(); ++y) {
        for (int x = 0; x < recolored.width(); ++x) {
            auto pixel = recolored.pixelColor(x, y);
            if (pixel.alpha() == 0) {
                continue;
            }
            pixel.setRgb(color.red(), color.green(), color.blue(),
                         pixel.alpha());
            recolored.setPixelColor(x, y, pixel);
        }
    }
    return recolored;
}

QImage transition_genesis_cram_image(const QImage& image,
                                     const std::uint8_t passes_completed,
                                     const bool toward_black) {
    if (image.isNull() || (toward_black && passes_completed == 0U)) {
        return image;
    }

    auto transitioned = image;
    for (int y = 0; y < transitioned.height(); ++y) {
        for (int x = 0; x < transitioned.width(); ++x) {
            auto pixel = transitioned.pixelColor(x, y);
            if (pixel.alpha() == 0) {
                continue;
            }
            const auto transition_channel = [passes_completed, toward_black](
                                                const int component,
                                                const bool green) {
                const auto source_level = gpgx_level_for_component(
                    component, green);
                const auto level = source_level >= 0
                    ? source_level
                    : static_cast<int>(
                          (static_cast<unsigned>(component) * 7U + 127U) /
                          255U);
                const auto bounded_passes = static_cast<int>(
                    std::min<std::uint8_t>(passes_completed, 7U));
                const auto result_level = toward_black
                    ? std::max(0, level - bounded_passes)
                    : std::min(level, bounded_passes);
                return gpgx_rgb565_component(result_level, green);
            };
            pixel.setRed(transition_channel(pixel.red(), false));
            pixel.setGreen(transition_channel(pixel.green(), true));
            pixel.setBlue(transition_channel(pixel.blue(), false));
            transitioned.setPixelColor(x, y, pixel);
        }
    }
    return transitioned;
}

QImage transition_genesis_cram_image_from_level(
    const QImage& image, const std::uint8_t start_level,
    const std::uint8_t passes_completed) {
    if (image.isNull()) {
        return image;
    }
    const auto step = [start_level, passes_completed](const int expanded,
                                                       const bool green) {
        const auto source_level = gpgx_level_for_component(expanded, green);
        const auto target = source_level >= 0
            ? source_level
            : static_cast<int>(
                  (static_cast<unsigned>(expanded) * 7U + 127U) / 255U);
        const auto start = static_cast<int>(std::min<std::uint8_t>(start_level, 7U));
        const auto passes = static_cast<int>(passes_completed);
        const auto level = target >= start ? std::min(target, start + passes)
                                           : std::max(target, start - passes);
        return gpgx_rgb565_component(level, green);
    };
    auto transitioned = image;
    for (int y = 0; y < transitioned.height(); ++y) {
        for (int x = 0; x < transitioned.width(); ++x) {
            auto pixel = transitioned.pixelColor(x, y);
            if (pixel.alpha() == 0) {
                continue;
            }
            pixel.setRed(step(pixel.red(), false));
            pixel.setGreen(step(pixel.green(), true));
            pixel.setBlue(step(pixel.blue(), false));
            transitioned.setPixelColor(x, y, pixel);
        }
    }
    return transitioned;
}

void make_field_hud_layer_transparent_outside_chrome(QImage& image) {
    // The extracted HUD plane bakes its color-zero cells as alpha 0
    // everywhere, including the six-tile chrome column at x=272. On the
    // Genesis the chrome's black panel cells are the VDP backdrop showing
    // through the transparent plane, so on the host the chrome column must
    // be opaque over the map while the 272px playfield stays transparent.
    constexpr int kFieldHudChromeX = 320 - (6 * 8);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            auto pixel = image.pixelColor(x, y);
            if (!is_black(pixel)) {
                continue;
            }
            if (x >= kFieldHudChromeX) {
                pixel.setAlpha(255);
            } else {
                pixel.setAlpha(0);
            }
            image.setPixelColor(x, y, pixel);
        }
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
