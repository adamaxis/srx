#include "vdp_model.hpp"

#include "genesis_image_compositor.hpp"

#include <QPainter>

#include <algorithm>

namespace shadowrun::ghidra_conversion::qt_runtime {
namespace {

std::size_t name_table_index(const std::size_t x, const std::size_t y) {
    return (y % VdpModel::kNameTableHeight) * VdpModel::kNameTableWidth +
           (x % VdpModel::kNameTableWidth);
}

std::size_t cram_index(const std::uint8_t palette,
                       const std::uint8_t color) {
    return (static_cast<std::size_t>(palette & 0x03U) *
            VdpModel::kCramColorsPerPalette) +
           static_cast<std::size_t>(color & 0x0FU);
}

int wrap_plane_coordinate(const int value, const int extent) {
    return static_cast<int>(static_cast<std::uint16_t>(value) &
                            static_cast<std::uint16_t>(extent - 1));
}

} // namespace

VdpNameTableEntry VdpNameTableEntry::decode(const std::uint16_t word) noexcept {
    return {(word & 0x8000U) != 0U,
            static_cast<std::uint8_t>((word >> 13U) & 0x03U),
            (word & 0x1000U) != 0U,
            (word & 0x0800U) != 0U,
            static_cast<std::uint16_t>(word & 0x07FFU)};
}

std::uint16_t VdpNameTableEntry::encode() const noexcept {
    return static_cast<std::uint16_t>(
        (priority ? 0x8000U : 0U) |
        ((static_cast<std::uint16_t>(palette) & 0x03U) << 13U) |
        (vertical_flip ? 0x1000U : 0U) |
        (horizontal_flip ? 0x0800U : 0U) |
        (tile_index & 0x07FFU));
}

void VdpModel::clear() noexcept {
    for (auto& pattern : patterns_) {
        pattern.fill(0U);
    }
    cram_.fill(0U);
    plane_a_.fill(0U);
    plane_b_.fill(0U);
    window_.fill(0U);
    scroll_ = {};
    backdrop_cram_index_ = 0U;
}

bool VdpModel::set_pattern(const std::uint16_t tile_index,
                           const Pattern& pattern_pixels) noexcept {
    if (tile_index >= kPatternCount) {
        return false;
    }
    patterns_[tile_index] = pattern_pixels;
    for (auto& color : patterns_[tile_index]) {
        color = static_cast<std::uint8_t>(color & 0x0FU);
    }
    return true;
}

const VdpModel::Pattern& VdpModel::pattern(
    const std::uint16_t tile_index) const noexcept {
    return patterns_[tile_index & 0x07FFU];
}

bool VdpModel::set_cram_word(const std::uint8_t palette,
                             const std::uint8_t color,
                             const std::uint16_t word) noexcept {
    if (palette >= kCramPaletteCount || color >= kCramColorsPerPalette) {
        return false;
    }
    cram_[cram_index(palette, color)] =
        static_cast<std::uint16_t>(word & 0x0FFFU);
    return true;
}

std::uint16_t VdpModel::cram_word(const std::uint8_t palette,
                                 const std::uint8_t color) const noexcept {
    return cram_[cram_index(palette, color)];
}

void VdpModel::set_name_word(const VdpPlane plane, const std::size_t x,
                             const std::size_t y,
                             const std::uint16_t word) noexcept {
    name_table(plane)[name_table_index(x, y)] = word;
}

std::uint16_t VdpModel::name_word(const VdpPlane plane,
                                  const std::size_t x,
                                  const std::size_t y) const noexcept {
    return name_table(plane)[name_table_index(x, y)];
}

VdpModel::NameTable& VdpModel::name_table(const VdpPlane plane) noexcept {
    switch (plane) {
    case VdpPlane::plane_a:
        return plane_a_;
    case VdpPlane::plane_b:
        return plane_b_;
    case VdpPlane::window:
        return window_;
    }
    return plane_a_;
}

const VdpModel::NameTable& VdpModel::name_table(
    const VdpPlane plane) const noexcept {
    switch (plane) {
    case VdpPlane::plane_a:
        return plane_a_;
    case VdpPlane::plane_b:
        return plane_b_;
    case VdpPlane::window:
        return window_;
    }
    return plane_a_;
}

void VdpModel::set_scroll(const VdpScrollState& scroll) noexcept {
    scroll_ = scroll;
}

void VdpModel::set_plane_b_vertical_scroll_offset(
    const std::uint16_t ffdd0e) noexcept {
    scroll_.plane_b_vertical = static_cast<std::int16_t>(ffdd0e);
}

void VdpModel::set_backdrop_cram_index(const std::uint8_t index) noexcept {
    backdrop_cram_index_ = static_cast<std::uint8_t>(index & 0x3FU);
}

void VdpModel::paint_plane(QImage& output, const VdpPlane plane,
                           const VdpScrollState& scroll,
                           const int priority_pass) const {
    if (plane == VdpPlane::window) {
        // The field window is positioned independently of the scrolling map.
        // The caller can leave cells outside the active window transparent.
    }
    const auto& names = name_table(plane);
    const auto horizontal = plane == VdpPlane::plane_a
        ? scroll.plane_a_horizontal
        : plane == VdpPlane::plane_b ? scroll.plane_b_horizontal : 0;
    const auto vertical = plane == VdpPlane::plane_a
        ? scroll.plane_a_vertical
        : plane == VdpPlane::plane_b ? scroll.plane_b_vertical : 0;
    auto* pixels = reinterpret_cast<QRgb*>(output.bits());
    const auto stride = output.bytesPerLine() / static_cast<int>(sizeof(QRgb));

    for (int screen_y = 0; screen_y < kDisplayHeight; ++screen_y) {
        const auto source_y = plane == VdpPlane::window
            ? screen_y
            : wrap_plane_coordinate(screen_y + vertical, 256);
        const auto tile_y = static_cast<std::size_t>(source_y / 8);
        const auto in_tile_y = static_cast<std::size_t>(source_y & 7);
        for (int screen_x = 0; screen_x < kDisplayWidth; ++screen_x) {
            const auto source_x = plane == VdpPlane::window
                ? screen_x
                : wrap_plane_coordinate(screen_x + horizontal, 512);
            const auto tile_x = static_cast<std::size_t>(source_x / 8);
            const auto in_tile_x = static_cast<std::size_t>(source_x & 7);
            const auto entry = VdpNameTableEntry::decode(
                names[name_table_index(tile_x, tile_y)]);
            if (priority_pass >= 0 &&
                entry.priority != (priority_pass != 0)) {
                continue;
            }
            const auto pattern_x = entry.horizontal_flip
                ? 7U - in_tile_x
                : in_tile_x;
            const auto pattern_y = entry.vertical_flip
                ? 7U - in_tile_y
                : in_tile_y;
            const auto color = patterns_[entry.tile_index][pattern_y * 8U +
                                                           pattern_x];
            if (color == 0U) {
                continue;
            }
            const auto word = cram_[cram_index(entry.palette, color)];
            const auto rgb = genesis_cram_word_to_color(word).rgb();
            pixels[screen_y * stride + screen_x] = rgb;
        }
    }
}

void VdpModel::compose(QImage& output,
                       const SpritePaintCallback& low_sprites,
                       const SpritePaintCallback& high_sprites,
                       const VdpSatScrollFrame* latched_frame) const {
    if (output.size() != QSize(kDisplayWidth, kDisplayHeight) ||
        output.format() != QImage::Format_ARGB32_Premultiplied) {
        output = QImage(kDisplayWidth, kDisplayHeight,
                        QImage::Format_ARGB32_Premultiplied);
    }
    const auto scroll = latched_frame != nullptr ? latched_frame->scroll
                                                  : scroll_;
    const auto backdrop = genesis_cram_word_to_color(
        cram_[static_cast<std::size_t>(backdrop_cram_index_)]).rgb();
    output.fill(backdrop);

    // Genesis VDP priority order: backdrop, B low, A low, low sprites,
    // B high, A high, window, high sprites [C: VDP priority rules].
    paint_plane(output, VdpPlane::plane_b, scroll, 0);
    paint_plane(output, VdpPlane::plane_a, scroll, 0);
    if (low_sprites) {
        QPainter painter(&output);
        low_sprites(painter);
    }
    paint_plane(output, VdpPlane::plane_b, scroll, 1);
    paint_plane(output, VdpPlane::plane_a, scroll, 1);
    paint_plane(output, VdpPlane::window, scroll, -1);
    if (high_sprites) {
        QPainter painter(&output);
        high_sprites(painter);
    }
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
