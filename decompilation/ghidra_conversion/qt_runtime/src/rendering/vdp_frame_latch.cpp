#include "vdp_frame_latch.hpp"

#include <utility>

namespace shadowrun::ghidra_conversion::qt_runtime {

void VdpFrameLatch::stage_for_vblank(VdpSatScrollFrame frame) {
    pending_frame_ = std::move(frame);
}

bool VdpFrameLatch::latch_vblank() {
    if (!pending_frame_.has_value()) {
        return false;
    }
    displayed_frame_ = std::move(*pending_frame_);
    pending_frame_.reset();
    has_displayed_frame_ = true;
    return true;
}

} // namespace shadowrun::ghidra_conversion::qt_runtime
