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

void make_genesis_sprite_color_zero_transparent(QImage& image) {
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width(); ++x) {
            clear_black_pixel(image, x, y);
        }
    }
}

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
    constexpr int kFieldHudChromeX = 320 - (6 * 8);
    for (int y = 0; y < image.height(); ++y) {
        for (int x = 0; x < image.width() && x < kFieldHudChromeX; ++x) {
            clear_black_pixel(image, x, y);
        }
    }
}

bool genesis_alias_uses_sprite_transparency(
    const std::string_view alias) noexcept {
    return alias == "boot.guy" || alias == "boot.fasa" ||
           alias.rfind("boot.sega.lightning.", 0) == 0 ||
           alias.rfind("boot.sega.strobe.", 0) == 0 ||
           alias == "title.logo" || alias == "title.text" ||
           alias == "menu.startup.cursor" ||
           alias.rfind("menu.field.runner.portrait.", 0) == 0 ||
           alias.rfind("title.sprite.", 0) == 0 ||
           alias.rfind("intro.sprites.", 0) == 0 ||
           alias.rfind("intro.background.patch.", 0) == 0 ||
           alias == "intro.aircraft.plane.a";
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
