#include "vdp_sprite_limits.hpp"

#include <algorithm>
#include <array>
#include <cstdint>

namespace shadowrun::ghidra_conversion::qt_runtime {

std::vector<VdpSpritePieceVisibility> apply_vdp_sprite_limits(
    const std::vector<VdpSpritePiece>& sat_ordered_pieces) {
    std::vector<VdpSpritePieceVisibility> result(
        sat_ordered_pieces.size());
    std::array<int, kVdpH40DisplayHeight> sprites_per_scanline{};
    std::array<int, kVdpH40DisplayHeight> pixels_per_scanline{};

    const auto considered = std::min(sat_ordered_pieces.size(),
                                     kVdpH40SpriteLimit);
    for (std::size_t sat_index = 0U; sat_index < considered; ++sat_index) {
        auto& visibility = result[sat_index];
        visibility.within_total_limit = true;
        const auto& piece = sat_ordered_pieces[sat_index];
        if (piece.width <= 0 || piece.height <= 0) {
            continue;
        }

        const auto left = static_cast<std::int64_t>(piece.x);
        const auto right = left + static_cast<std::int64_t>(piece.width);
        const auto visible_left = std::clamp<std::int64_t>(
            left, 0, kVdpH40DisplayWidth);
        const auto visible_right = std::clamp<std::int64_t>(
            right, 0, kVdpH40DisplayWidth);
        const auto visible_width = static_cast<int>(
            std::max<std::int64_t>(0, visible_right - visible_left));

        const auto top = static_cast<std::int64_t>(piece.y);
        const auto bottom = top + static_cast<std::int64_t>(piece.height);
        const auto first_scanline = std::max<std::int64_t>(top, 0);
        const auto final_scanline = std::min<std::int64_t>(
            bottom, kVdpH40DisplayHeight);
        for (auto scanline = first_scanline;
             scanline < final_scanline; ++scanline) {
            const auto line = static_cast<std::size_t>(scanline);
            if (sprites_per_scanline[line] >=
                    kVdpH40SpriteScanlineLimit ||
                pixels_per_scanline[line] + visible_width >
                    kVdpH40SpritePixelScanlineLimit) {
                continue;
            }
            ++sprites_per_scanline[line];
            pixels_per_scanline[line] += visible_width;
            visibility.visible_scanlines.push_back(
                static_cast<int>(scanline));
        }
    }

    return result;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
