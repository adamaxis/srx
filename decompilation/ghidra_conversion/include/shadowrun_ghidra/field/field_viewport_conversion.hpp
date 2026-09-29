#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct FieldViewportState {
    std::uint16_t normal_render_pending = 0; // FFE100
    std::uint16_t alternate_render_pending = 0; // FFE102
    std::uint16_t render_mask = 0; // FFDF58
    std::uint16_t plane_a_offset = 0; // FFDD14
    std::uint16_t plane_b_offset = 0; // FFDD16
    std::uint32_t plane_a_destination = 0; // *FFDDDE
    std::uint32_t plane_b_destination = 0; // *FFDDE6
    std::uint16_t selected_plane_offset = 0; // D5
    std::uint32_t selected_destination = 0; // A0
    std::uint32_t selected_tile_table = 0; // A1
};

constexpr std::uint32_t kFieldNormalTileTable = 0x00FF2700U;
constexpr std::uint32_t kFieldAlternateTileTable = 0x00FF4700U;

using FieldViewportRenderBoundary = std::function<void()>;

// ROM 0x2C92: CLR FFE100, D5=FFDD14, A0=*FFDDDE, A1=$FF2700, JSR 0x3D18.
void render_normal_field_viewport(
    FieldViewportState& state,
    const FieldViewportRenderBoundary& render_viewport);

// ROM 0x2CB2: CLR FFE102, D5=FFDD16, A0=*FFDDE6, A1=$FF4700, FFDF58=#0x8000,
// JSR 0x3D18, CLR FFDF58.
void render_alternate_field_viewport(
    FieldViewportState& state,
    const FieldViewportRenderBoundary& render_viewport);

} // namespace shadowrun::ghidra_conversion
