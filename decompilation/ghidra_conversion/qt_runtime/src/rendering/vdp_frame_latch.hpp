#pragma once

#include "vdp_sprite_limits.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

struct VdpScrollState {
    std::int16_t plane_a_horizontal = 0;
    std::int16_t plane_a_vertical = 0;
    std::int16_t plane_b_horizontal = 0;
    std::int16_t plane_b_vertical = 0;
};

struct VdpSatScrollFrame {
    std::vector<VdpSpritePiece> sat_ordered_pieces;
    VdpScrollState scroll;
};

// Hold the source-built SAT and scroll values until the next VBlank. ROM
// 0x1872 uploads SAT before applying H/V scroll, so those values become one
// coherent displayed frame at that boundary [C].
class VdpFrameLatch final {
public:
    void stage_for_vblank(VdpSatScrollFrame frame);
    bool latch_vblank();

    const VdpSatScrollFrame& displayed_frame() const noexcept {
        return displayed_frame_;
    }
    const VdpSatScrollFrame* pending_frame() const noexcept {
        return pending_frame_ ? &*pending_frame_ : nullptr;
    }
    bool has_pending_frame() const noexcept {
        return pending_frame_.has_value();
    }
    bool has_displayed_frame() const noexcept {
        return has_displayed_frame_;
    }

private:
    VdpSatScrollFrame displayed_frame_;
    std::optional<VdpSatScrollFrame> pending_frame_;
    bool has_displayed_frame_ = false;
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
