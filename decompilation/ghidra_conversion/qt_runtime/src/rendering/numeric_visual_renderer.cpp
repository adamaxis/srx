#include "numeric_visual_renderer.hpp"

#include "shadowrun_ghidra/rendering/decimal_glyph_conversion.hpp"

#include <algorithm>
#include <cstdint>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

constexpr int kGlyphWidth = 8;
constexpr int kGlyphHeight = 8;
constexpr int kGlyphColumns = 16;
constexpr unsigned char kFirstBitmapFontCode = 0x20U;
constexpr unsigned char kLastBitmapFontCode = 0x7EU;

QImage tinted_glyph(const QImage& atlas,
                    const int source_x,
                    const int source_y,
                    const QColor& color) {
    QImage glyph(kGlyphWidth, kGlyphHeight,
                 QImage::Format_ARGB32_Premultiplied);
    glyph.fill(Qt::transparent);
    for (int y = 0; y < kGlyphHeight; ++y) {
        for (int x = 0; x < kGlyphWidth; ++x) {
            const auto source = atlas.pixelColor(source_x + x, source_y + y);
            const auto coverage = std::max(
                {source.red(), source.green(), source.blue()});
            if (source.alpha() == 0 || coverage == 0) {
                continue;
            }
            const auto alpha = static_cast<int>(
                static_cast<std::uint32_t>(source.alpha()) *
                static_cast<std::uint32_t>(coverage) / 255U);
            glyph.setPixelColor(x, y,
                                QColor(color.red(), color.green(),
                                       color.blue(), alpha));
        }
    }
    return glyph;
}

} // namespace

QImage render_numeric_visual(const QImage& font_atlas,
                             const std::uint32_t value,
                             const bool group_thousands,
                             const std::optional<QColor> tint) {
    if (font_atlas.isNull() || font_atlas.width() < kGlyphColumns * kGlyphWidth ||
        font_atlas.height() < kGlyphHeight) {
        return {};
    }

    const auto visual = build_numeric_visual(value, group_thousands);
    const auto width = static_cast<int>(visual.pixel_width(kGlyphWidth));
    if (width <= 0) {
        return {};
    }

    QImage result(width, kGlyphHeight,
                  QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);
    QPainter painter(&result);
    for (std::size_t index = 0; index < visual.glyphs.size(); ++index) {
        const auto code = static_cast<unsigned char>(visual.glyphs[index]);
        if (code < kFirstBitmapFontCode || code > kLastBitmapFontCode) {
            continue;
        }
        const auto glyph = static_cast<int>(code - kFirstBitmapFontCode);
        const auto source_x = (glyph % kGlyphColumns) * kGlyphWidth;
        const auto source_y = (glyph / kGlyphColumns) * kGlyphHeight;
        const auto destination_x = static_cast<int>(index) * kGlyphWidth;
        if (source_y + kGlyphHeight > font_atlas.height()) {
            continue;
        }
        if (tint.has_value()) {
            painter.drawImage(destination_x, 0,
                              tinted_glyph(font_atlas, source_x, source_y,
                                           *tint));
        } else {
            painter.drawImage(destination_x, 0, font_atlas,
                              source_x, source_y, kGlyphWidth, kGlyphHeight);
        }
    }
    return result;
}

void draw_numeric_visual(QPainter& painter,
                         const QImage& font_atlas,
                         const int x,
                         const int y,
                         const std::uint32_t value,
                         const NumericVisualAlignment alignment,
                         const bool group_thousands,
                         const std::optional<QColor> tint) {
    const auto visual = render_numeric_visual(
        font_atlas, value, group_thousands, tint);
    if (visual.isNull()) {
        return;
    }
    const auto left = alignment == NumericVisualAlignment::right
                          ? x - visual.width()
                          : x;
    painter.drawImage(left, y, visual);
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
