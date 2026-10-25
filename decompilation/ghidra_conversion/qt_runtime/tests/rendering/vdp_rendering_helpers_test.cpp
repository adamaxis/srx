#include "vdp_frame_latch.hpp"
#include "vdp_sprite_limits.hpp"

#include <iostream>
#include <vector>

namespace {

bool expect(const bool condition, const char* message) {
    if (!condition) {
        std::cerr << message << '\n';
        return false;
    }
    return true;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion::qt_runtime;

    std::vector<VdpSpritePiece> total_limit_pieces;
    total_limit_pieces.reserve(kVdpH40SpriteLimit + 1U);
    for (int index = 0; index < static_cast<int>(kVdpH40SpriteLimit + 1U);
         ++index) {
        total_limit_pieces.push_back({0, index, 8, 1});
    }
    const auto total_limit = apply_vdp_sprite_limits(total_limit_pieces);
    if (!expect(total_limit.size() == kVdpH40SpriteLimit + 1U,
                "the visibility result must preserve every SAT entry") ||
        !expect(total_limit[kVdpH40SpriteLimit - 1U].within_total_limit,
                "the eightieth SAT entry must be considered") ||
        !expect(!total_limit[kVdpH40SpriteLimit]
                     .within_total_limit &&
                    total_limit[kVdpH40SpriteLimit]
                        .visible_scanlines.empty(),
                "SAT entries after the first 80 must be dropped")) {
        return 1;
    }

    std::vector<VdpSpritePiece> sprite_count_pieces(
        kVdpH40SpriteScanlineLimit + 1,
        VdpSpritePiece{0, 12, 8, 1});
    const auto sprite_count = apply_vdp_sprite_limits(sprite_count_pieces);
    for (std::size_t index = 0U;
         index < static_cast<std::size_t>(kVdpH40SpriteScanlineLimit);
         ++index) {
        if (!expect(sprite_count[index].visible_scanlines ==
                        std::vector<int>{12},
                    "lower SAT indices must win the 20-sprite line limit")) {
            return 1;
        }
    }
    if (!expect(sprite_count[kVdpH40SpriteScanlineLimit]
                    .visible_scanlines.empty(),
                "the twenty-first sprite on a line must be dropped")) {
        return 1;
    }

    const std::vector<VdpSpritePiece> pixel_pieces{
        {0, 20, 256, 1}, {0, 20, 64, 2}, {0, 20, 8, 1}};
    const auto pixel_limit = apply_vdp_sprite_limits(pixel_pieces);
    if (!expect(pixel_limit[0].visible_scanlines ==
                    std::vector<int>{20},
                "the first sprite must survive the pixel-budget line") ||
        !expect(pixel_limit[1].visible_scanlines ==
                    (std::vector<int>{20, 21}),
                "a sprite may survive on its next line after an earlier line fills") ||
        !expect(pixel_limit[2].visible_scanlines.empty(),
                "later pieces must lose when a scanline reaches 320 pixels")) {
        return 1;
    }

    std::vector<VdpSpritePiece> clipped_pixel_pieces(
        17U, VdpSpritePiece{300, 30, 32, 1});
    const auto clipped_pixel_limit =
        apply_vdp_sprite_limits(clipped_pixel_pieces);
    if (!expect(clipped_pixel_limit[15].visible_scanlines ==
                    std::vector<int>{30},
                "visible width must be clipped at the H40 right edge") ||
        !expect(clipped_pixel_limit[16].visible_scanlines.empty(),
                "clipped sprite pixels must still obey the 320-pixel line cap")) {
        return 1;
    }

    VdpFrameLatch latch;
    if (!expect(!latch.latch_vblank() && !latch.has_displayed_frame(),
                "an empty VBlank must leave the initial display state alone")) {
        return 1;
    }
    VdpSatScrollFrame first_frame;
    first_frame.sat_ordered_pieces.push_back({4, 8, 16, 16});
    first_frame.scroll = {10, 20, 30, 40};
    latch.stage_for_vblank(first_frame);
    if (!expect(latch.has_pending_frame() &&
                    !latch.has_displayed_frame(),
                "staged SAT and scroll must wait for VBlank") ||
        !expect(latch.latch_vblank() && latch.has_displayed_frame() &&
                    !latch.has_pending_frame(),
                "VBlank must publish the staged frame atomically") ||
        !expect(latch.displayed_frame().sat_ordered_pieces.size() == 1U &&
                    latch.displayed_frame().scroll.plane_b_vertical == 40,
                "the latched frame must include both SAT and scroll")) {
        return 1;
    }
    VdpSatScrollFrame next_frame;
    next_frame.sat_ordered_pieces.push_back({12, 16, 8, 8});
    next_frame.scroll = {11, 21, 31, 41};
    latch.stage_for_vblank(next_frame);
    if (!expect(latch.displayed_frame().scroll.plane_a_horizontal == 10,
                "the prior frame remains displayed before the next VBlank") ||
        !expect(latch.latch_vblank() &&
                    latch.displayed_frame().scroll.plane_a_horizontal == 11 &&
                    latch.displayed_frame().scroll.plane_b_vertical == 41,
                "the next VBlank must expose the new SAT and scroll together")) {
        return 1;
    }
    return 0;
}
