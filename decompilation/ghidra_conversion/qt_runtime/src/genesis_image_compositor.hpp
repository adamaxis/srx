#pragma once

#include <QImage>

#include <cstdint>
#include <string_view>

namespace shadowrun::ghidra_conversion::qt_runtime {

// Direct BMP extraction preserves Genesis color index zero as RGB black.
// Sprite/object records use that index as a transparent pixel; full-plane
// assets do not.  Keep the policy in the host compositor instead of making
// each scene renderer guess from the bitmap's shape.
void make_genesis_sprite_color_zero_transparent(QImage& image);

// Apply a Genesis palette fade to an extracted image without changing its
// color-zero alpha mask. The source CRAM helpers move 3-bit components in
// steps; the presentation boundary carries the equivalent bounded weight.
QImage scale_genesis_image_rgb(const QImage& image, std::uint16_t weight,
                               std::uint16_t maximum = 256U);

// The field HUD extraction is a full 320x224 view of the source HUD layer.
// 0x334C installs a six-tile (48px) right-hand chrome column at x=272, while
// the black pixels over the 272px playfield are Plane-A color-zero holes.
void make_field_hud_layer_transparent_outside_chrome(QImage& image);

// Return true for extracted aliases whose catalog record is a SAT/object
// sprite rather than a full-screen plane/background.
bool genesis_alias_uses_sprite_transparency(std::string_view alias) noexcept;

} // namespace shadowrun::ghidra_conversion::qt_runtime
