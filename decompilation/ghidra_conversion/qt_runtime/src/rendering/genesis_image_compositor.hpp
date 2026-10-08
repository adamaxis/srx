#pragma once

#include <QImage>

#include <cstdint>
#include <string_view>

namespace shadowrun::ghidra_conversion::qt_runtime {

// Apply a Genesis palette fade to an extracted image without changing its
// color-zero alpha mask. The source CRAM helpers move 3-bit components in
// steps; the presentation boundary carries the equivalent bounded weight.
QImage scale_genesis_image_rgb(const QImage& image, std::uint16_t weight,
                               std::uint16_t maximum = 256U);

// Replace each visible pixel with one expanded 12-bit Genesis CRAM color.
// This is used only while an intro palette bank has all fifteen nonzero
// entries set to the same color; source color-zero transparency is retained.
QImage recolor_genesis_image_to_cram_word(const QImage& image,
                                          std::uint16_t color_word);

// Render one of ROM 0x1088/0x10D6's CRAM palette passes. Extracted Genesis
// colors are expanded 3-bit RGB, so the native image can reproduce a fade by
// stepping each component in its original 0..7 range. Alpha stays unchanged.
QImage transition_genesis_cram_image(const QImage& image,
                                     std::uint8_t passes_completed,
                                     bool toward_black);

// The field HUD extraction is a full 320x224 view of the source HUD layer.
// 0x334C installs a six-tile (48px) right-hand chrome column at x=272. The
// plane's color-zero cells are baked as alpha 0 everywhere, so the host
// restores the chrome column's black panel (the Genesis VDP backdrop) while
// leaving the 272px playfield as Plane-A color-zero holes.
void make_field_hud_layer_transparent_outside_chrome(QImage& image);

} // namespace shadowrun::ghidra_conversion::qt_runtime
