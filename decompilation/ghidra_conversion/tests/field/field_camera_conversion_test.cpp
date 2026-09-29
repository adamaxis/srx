#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/field/field_camera_conversion.hpp"

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    if (!expect(classify_field_camera_axis_step(1000, 1000 + 0x100, 0) == 0 &&
                classify_field_camera_axis_step(1000, 1000 + 0x180, 0) == 1 &&
                classify_field_camera_axis_step(1000, 1000 + 0x180, 2) == 2 &&
                classify_field_camera_axis_step(1000, 1000 + 0x300, 0) == 2 &&
                classify_field_camera_axis_step(1000, 1000 - 0x180, -1) == -1 &&
                classify_field_camera_axis_step(1000 + 0x100, 1000, -1) == -1)) {
        return 1;
    }

    FieldCameraFollowState blocked{};
    blocked.camera_x = 0x0100U;
    blocked.camera_y = 0x0200U;
    blocked.maximum_x = 0x4000U;
    blocked.maximum_y = 0x4000U;
    blocked.layer_x_dirty = true;
    const auto blocked_result =
        follow_field_camera(blocked, 0x2000U, 0x2000U, 0, false);
    if (!expect(blocked_result.camera_x == blocked.camera_x &&
                blocked_result.camera_y == blocked.camera_y &&
                blocked_result.horizontal_step != 0)) {
        return 2;
    }

    FieldCameraFollowState bounded{};
    bounded.camera_x = 0x1000U;
    bounded.maximum_x = 0x1000U;
    bounded.maximum_y = 0x4000U;
    bounded.aligned_x = 0x1000U;
    const auto held =
        follow_field_camera(bounded, 0x4000U, bounded.camera_y, 0, false);
    if (!expect(held.camera_x == 0x1000U)) {
        return 3;
    }

    FieldCameraFollowState step{};
    step.camera_x = 0x0100U;
    step.camera_y = 0x0800U;
    step.maximum_x = 0x2000U;
    step.maximum_y = 0x2000U;
    step.aligned_x = 0x0100U;
    step.aligned_y = 0x0800U;
    // Heading 0 has no horizontal lookahead. Center X is 0x0980, so an
    // entity at center+0x300 requests the fast positive step.
    const auto chased = follow_field_camera(
        step, static_cast<std::uint16_t>(0x0980U + 0x300U), 0x0800U, 0, false);
    if (!expect(chased.horizontal_step == 2 &&
                chased.camera_x == static_cast<std::uint16_t>(0x0100U + 0x20U) &&
                chased.horizontal_bias_a == -2 &&
                chased.horizontal_bias_b == -2)) {
        return 4;
    }

    FieldCameraFollowState overview{};
    overview.maximum_x = 0x2000U;
    overview.maximum_y = 0x2000U;
    const auto fixed = follow_field_camera(overview, 0, 0, 0, true);
    if (!expect(!fixed.chase_stopped &&
                (fixed.camera_x != 0U || fixed.camera_y != 0U))) {
        return 5;
    }
    FieldCameraFollowState dead = overview;
    dead.camera_x = 0x0600U;
    const auto stopped = follow_field_camera(dead, 0, 0, 0, true);
    if (!expect(stopped.chase_stopped && stopped.camera_x == 0x0600U &&
                stopped.camera_y == 0U)) {
        return 6;
    }

    // ROM 0x15DCC heading 0: D2 from 0x15E16 (C++ vertical), D3 from 0x15E56
    // (C++ horizontal, 0). ROM 0x3A50 adds D3 to entity X and D2 to entity Y.
    const auto heading0 = transform_entity_heading_vector(0, 0x03C0);
    if (!expect(heading0.horizontal == 0 && heading0.vertical != 0)) {
        return 7;
    }
    FieldCameraFollowState origin{};
    origin.camera_x = 0x0A00U;
    origin.camera_y = 0x0A00U;
    origin.maximum_x = 0x4000U;
    origin.maximum_y = 0x4000U;
    origin.aligned_x = 0x0A00U;
    origin.aligned_y = 0x0A00U;
    const auto north = follow_field_camera(
        origin,
        static_cast<std::uint16_t>(origin.camera_x + 0x0880U),
        static_cast<std::uint16_t>(origin.camera_y + 0x0700U),
        0,
        false);
    if (!expect(north.horizontal_step == 0 && north.vertical_step < 0 &&
                north.camera_x == origin.camera_x &&
                north.camera_y != origin.camera_y)) {
        return 8;
    }
    return 0;
}
