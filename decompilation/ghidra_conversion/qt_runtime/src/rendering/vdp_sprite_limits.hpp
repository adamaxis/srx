#pragma once

#include <cstddef>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

inline constexpr std::size_t kVdpH40SpriteLimit = 80U;
inline constexpr int kVdpH40SpriteScanlineLimit = 20;
inline constexpr int kVdpH40SpritePixelScanlineLimit = 320;
inline constexpr int kVdpH40DisplayWidth = 320;
inline constexpr int kVdpH40DisplayHeight = 224;

struct VdpSpritePiece {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;
};

// The same SAT piece can survive on some scanlines and be dropped on others.
// The row list is in display coordinates and preserves SAT priority by index.
struct VdpSpritePieceVisibility {
    bool within_total_limit = false;
    std::vector<int> visible_scanlines;
};

// Model the H40 SAT limits without owning rendering state. Input order is SAT
// order; the first 80 entries are considered, and lower indices win each
// scanline's 20-sprite / 320-visible-pixel budget [C: H40 VDP]. The field SAT
// order is produced by 0x4658 and the per-actor piece emitter at 0x622C.
// Coordinates are screen-space bounds after applying the SAT $80 origin.
std::vector<VdpSpritePieceVisibility> apply_vdp_sprite_limits(
    const std::vector<VdpSpritePiece>& sat_ordered_pieces);

} // namespace shadowrun::ghidra_conversion::qt_runtime
