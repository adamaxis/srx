#include "genesis_image_compositor.hpp"

#include <QColor>

#include <algorithm>
#include <string_view>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

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
    const auto expand_component = [](const std::uint16_t component) {
        return static_cast<int>((component << 5U) | (component << 2U) |
                                (component >> 1U));
    };
    // Genesis CRAM words are RRR0 GGG0 BBB0: red is bits 1..3, green 5..7,
    // and blue 9..11. The field palette upload at 0x1406 and the shared
    // 48-word copy at 0x566C2 establish these values as the field CRAM source
    // [C: VDP CRAM format; ROM 0x1406/0x566C2].
    return QColor(
        expand_component(static_cast<std::uint16_t>((color_word >> 1U) & 7U)),
        expand_component(static_cast<std::uint16_t>((color_word >> 5U) & 7U)),
        expand_component(static_cast<std::uint16_t>((color_word >> 9U) & 7U)));
}

QImage recolor_genesis_image_to_cram_word(const QImage& image,
                                          const std::uint16_t color_word) {
    if (image.isNull()) {
        return image;
    }

    // Their three-bit channels expand exactly to the same 0..255 values
    // used by the offline graphics importer.
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

    const auto transition_component = [passes_completed, toward_black](
                                          const int expanded_component) {
        const auto source_level = static_cast<int>(
            (static_cast<unsigned>(expanded_component) * 7U + 127U) / 255U);
        const auto bounded_passes = static_cast<int>(
            std::min<std::uint8_t>(passes_completed, 7U));
        const auto result_level = toward_black
            ? std::max(0, source_level - bounded_passes)
            : std::min(source_level, bounded_passes);
        return (result_level << 5) | (result_level << 2) |
               (result_level >> 1);
    };

    auto transitioned = image;
    for (int y = 0; y < transitioned.height(); ++y) {
        for (int x = 0; x < transitioned.width(); ++x) {
            auto pixel = transitioned.pixelColor(x, y);
            if (pixel.alpha() == 0) {
                continue;
            }
            pixel.setRed(transition_component(pixel.red()));
            pixel.setGreen(transition_component(pixel.green()));
            pixel.setBlue(transition_component(pixel.blue()));
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
    const auto step = [start_level, passes_completed](const int expanded) {
        const auto target = static_cast<int>(
            (static_cast<unsigned>(expanded) * 7U + 127U) / 255U);
        const auto start = static_cast<int>(std::min<std::uint8_t>(start_level, 7U));
        const auto passes = static_cast<int>(passes_completed);
        const auto level = target >= start ? std::min(target, start + passes)
                                           : std::max(target, start - passes);
        return (level << 5) | (level << 2) | (level >> 1);
    };
    auto transitioned = image;
    for (int y = 0; y < transitioned.height(); ++y) {
        for (int x = 0; x < transitioned.width(); ++x) {
            auto pixel = transitioned.pixelColor(x, y);
            if (pixel.alpha() == 0) {
                continue;
            }
            pixel.setRed(step(pixel.red()));
            pixel.setGreen(step(pixel.green()));
            pixel.setBlue(step(pixel.blue()));
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
