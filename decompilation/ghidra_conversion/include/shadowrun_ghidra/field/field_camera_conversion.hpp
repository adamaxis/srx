#pragma once

#include <cstdint>

namespace shadowrun::ghidra_conversion {

// ROM 0x3A90 and 0x3B1E. The center word is the camera plus 0x880 or 0x700.
// Gaps of 0x300 and 0x180 select a two-speed step, and the previous step
// holds the slower speed inside the inner band.
std::int8_t classify_field_camera_axis_step(std::uint16_t center,
                                            std::uint16_t target,
                                            std::int8_t previous_step);

struct FieldCameraFollowState {
    std::uint16_t camera_x = 0;
    std::uint16_t camera_y = 0;
    std::uint16_t minimum_x = 0;
    std::uint16_t maximum_x = 0;
    std::uint16_t minimum_y = 0;
    std::uint16_t maximum_y = 0;
    std::uint16_t aligned_x = 0;
    std::uint16_t aligned_y = 0;
    // Per-plane scroll-step biases for FFDD08/FFDD0C and FFDD0A/FFDD0E.
    // ROM 0x3C8E..0x3CFE advances each pair together; horizontal deltas are
    // negated while vertical deltas are positive [C].
    std::int16_t horizontal_bias_a = 0;
    std::int16_t horizontal_bias_b = 0;
    std::int16_t vertical_bias_a = 0;
    std::int16_t vertical_bias_b = 0;
    std::int8_t horizontal_step = 0;
    std::int8_t vertical_step = 0;
    bool layer_x_dirty = false;
    bool layer_y_dirty = false;
    bool interaction_locked = false;
    bool horizontal_edge = false;
    bool vertical_edge = false;
    bool chase_stopped = false;
};

// ROM 0x3A1C through 0x3C36. The ordinary path aims 0x3C0 units ahead of the
// entity heading. FFE186 == 3 uses the fixed target (0x600, 0) and the
// 0x3BAC direction table instead. Dirty layers and the interaction lock skip
// the camera write after the step is chosen.
FieldCameraFollowState follow_field_camera(FieldCameraFollowState state,
                                           std::uint16_t entity_x,
                                           std::uint16_t entity_y,
                                           std::uint16_t heading,
                                           bool fixed_overview);

} // namespace shadowrun::ghidra_conversion
