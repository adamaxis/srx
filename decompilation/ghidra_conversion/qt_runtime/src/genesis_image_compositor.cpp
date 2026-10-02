#include "genesis_image_compositor.hpp"

#include <QColor>

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

QImage scale_genesis_image_rgb(const QImage& image,
                               const std::uint16_t weight,
                               const std::uint16_t maximum) {
    if (maximum == 0U || weight >= maximum) {
        return image;
    }
    auto faded = image;
    for (int y = 0; y < faded.height(); ++y) {
        for (int x = 0; x < faded.width(); ++x) {
            auto pixel = faded.pixelColor(x, y);
            pixel.setRed(static_cast<int>(
                static_cast<std::uint32_t>(pixel.red()) * weight /
                maximum));
            pixel.setGreen(static_cast<int>(
                static_cast<std::uint32_t>(pixel.green()) * weight /
                maximum));
            pixel.setBlue(static_cast<int>(
                static_cast<std::uint32_t>(pixel.blue()) * weight /
                maximum));
            faded.setPixelColor(x, y, pixel);
        }
    }
    return faded;
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
