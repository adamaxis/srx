#pragma once

#include "vdp_model.hpp"
#include "shadowrun_ghidra/field/field_viewport_conversion.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace shadowrun::ghidra_conversion::qt_runtime {

// A row-major host view of source name words, resolving the A0/A1 data selected
// by 0x2C92 or 0x2CB2 into typed storage. Field maps are larger than the VDP's
// 64x32 ring, so the streamer copies the visible 44x31-tile window into the
// wrapped name table before compose() applies the pixel scroll.
struct PlaneTilemapSource {
    const std::vector<std::uint16_t>* name_words = nullptr;
    std::size_t width_tiles = 0U;
    std::size_t height_tiles = 0U;
    std::uint16_t source_x_pixels = 0U;
    std::uint16_t source_y_pixels = 0U;
};

class PlaneStreamer final {
public:
    explicit PlaneStreamer(VdpModel& vdp) noexcept : vdp_(vdp) {}

    // Ports 0x2C92/0x2CB2. These call the existing viewport conversion and
    // 0x3D18/0x3D4A row/column contracts; the alternate path applies the
    // FFDF58 XOR-priority rule [C].
    std::size_t stream_normal_viewport(const PlaneTilemapSource& source,
                                       FieldViewportState& state);
    std::size_t stream_alternate_viewport(const PlaneTilemapSource& source,
                                          FieldViewportState& state);

    // The four runtime writers at 0x406A/0x40CE (0x3D4A) and
    // 0x4136/0x41A2 (0x3DDE) set FFDF58=0x8000 around the stamp write;
    // 0x3D4A applies EORI #$8000 at 0x3DAE, while 0x3DDE uses the row uploader
    // at 0x3E14. The wrappers and priority inversion are confirmed [C]. The
    // host metatile payload is modeled as 4x4 name words (32x32 pixels) [H];
    // ROM upload counts are register-dependent and the writer is more general.
    std::size_t write_metatile(VdpPlane plane, std::size_t destination_x,
                               std::size_t destination_y,
                               const std::array<std::uint16_t, 16>& words);

private:
    VdpModel& vdp_;

    std::size_t stream_selected_plane(const PlaneTilemapSource& source,
                                      const FieldViewportState& state,
                                      VdpPlane plane,
                                      std::uint16_t priority_mask);
};

} // namespace shadowrun::ghidra_conversion::qt_runtime
