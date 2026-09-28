#include "shadowrun_ghidra/field/field_camera_conversion.hpp"

#include "shadowrun_ghidra/entities/entity_coordinate_conversion.hpp"
#include "shadowrun_ghidra/entities/movement_metric_conversion.hpp"

#include <array>

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::uint16_t kCameraCenterOffsetX = 0x880U;
constexpr std::uint16_t kCameraCenterOffsetY = 0x700U;
constexpr std::int16_t kCameraLookahead = 0x03C0;
constexpr std::uint16_t kFixedOverviewX = 0x0600U;
constexpr std::uint16_t kChaseDeadzone = 0x0020U;
constexpr std::uint16_t kChaseFastDistance = 0x0028U;

// ROM 0x4290. Eight heading slots, each an X word then a Y word.
constexpr std::array<std::array<std::int8_t, 2>, 8> kCameraDirectionSteps{{
    {{0, 1}},  {{-1, 1}}, {{-1, 0}}, {{-1, -1}},
    {{0, -1}}, {{1, -1}}, {{1, 0}},  {{1, 1}},
}};

std::uint16_t add_word(const std::uint16_t base, const std::int16_t delta) {
    return static_cast<std::uint16_t>(
        static_cast<std::int32_t>(base) + delta);
}

std::uint16_t nonnegative_sum(const std::uint16_t base,
                              const std::int16_t delta) {
    const auto sum = static_cast<std::int16_t>(add_word(base, delta));
    return sum < 0 ? 0U : static_cast<std::uint16_t>(sum);
}

void apply_camera_axis(std::uint16_t& camera,
                       const std::uint16_t minimum,
                       const std::uint16_t maximum,
                       const std::int8_t step,
                       const bool negate_bias,
                       std::int16_t& bias_a,
                       std::int16_t& bias_b,
                       std::uint16_t& aligned,
                       bool& edge) {
    if (step == 0) {
        return;
    }
    const auto shifted = static_cast<std::int16_t>(
        static_cast<std::uint16_t>(static_cast<std::uint16_t>(step) << 4U));
    const auto next = add_word(camera, shifted);
    const auto signed_next = static_cast<std::int16_t>(next);
    if (shifted < 0) {
        if (signed_next < static_cast<std::int16_t>(minimum)) {
            return;
        }
    } else if (signed_next > static_cast<std::int16_t>(maximum)) {
        return;
    }

    camera = next;
    const auto bias_delta =
        static_cast<std::int16_t>(negate_bias ? -step : step);
    bias_a = static_cast<std::int16_t>(bias_a + bias_delta);
    bias_b = static_cast<std::int16_t>(bias_b + bias_delta);
    const auto next_aligned = static_cast<std::uint16_t>(camera & 0xFF80U);
    if (next_aligned != aligned) {
        aligned = next_aligned;
        edge = true;
    }
}

void apply_camera_steps(FieldCameraFollowState& state) {
    if (state.layer_x_dirty || state.layer_y_dirty ||
        state.interaction_locked) {
        return;
    }
    apply_camera_axis(state.camera_x, state.minimum_x, state.maximum_x,
                      state.horizontal_step, true, state.horizontal_bias_a,
                      state.horizontal_bias_b, state.aligned_x,
                      state.horizontal_edge);
    apply_camera_axis(state.camera_y, state.minimum_y, state.maximum_y,
                      state.vertical_step, false, state.vertical_bias_a,
                      state.vertical_bias_b, state.aligned_y,
                      state.vertical_edge);
}

} // namespace

std::int8_t classify_field_camera_axis_step(const std::uint16_t center,
                                            const std::uint16_t target,
                                            const std::int8_t previous_step) {
    const auto toward_positive = center < target;
    const auto gap = static_cast<std::uint16_t>(
        toward_positive ? target - center : center - target);
    const auto fast = static_cast<std::int8_t>(toward_positive ? 2 : -2);
    const auto slow = static_cast<std::int8_t>(toward_positive ? 1 : -1);
    if (gap >= 0x300U) {
        return fast;
    }
    if (gap >= 0x180U) {
        return previous_step == fast ? fast : slow;
    }
    return previous_step == slow ? slow : 0;
}

FieldCameraFollowState follow_field_camera(FieldCameraFollowState state,
                                           const std::uint16_t entity_x,
                                           const std::uint16_t entity_y,
                                           const std::uint16_t heading,
                                           const bool fixed_overview) {
    state.horizontal_edge = false;
    state.vertical_edge = false;
    state.chase_stopped = false;

    if (fixed_overview) {
        const auto measurement = measure_relative_heading_and_distance(
            kFixedOverviewX, 0, state.camera_x, state.camera_y);
        const auto distance =
            static_cast<std::uint16_t>(measurement.distance);
        if (distance < kChaseDeadzone) {
            state.chase_stopped = true;
            return state;
        }
        const auto slot = static_cast<std::size_t>(
            (measurement.heading >> 13U) & 0x07U);
        auto step_x = kCameraDirectionSteps[slot][0];
        auto step_y = kCameraDirectionSteps[slot][1];
        const auto on_bound =
            state.camera_x == state.minimum_x ||
            state.camera_x == state.maximum_x ||
            state.camera_y == state.minimum_y ||
            state.camera_y == state.maximum_y;
        if (distance >= kChaseFastDistance && !on_bound) {
            step_x = static_cast<std::int8_t>(step_x * 2);
            step_y = static_cast<std::int8_t>(step_y * 2);
        }
        state.horizontal_step = step_x;
        state.vertical_step = step_y;
        apply_camera_steps(state);
        return state;
    }

    const auto lookahead =
        transform_entity_heading_vector(heading, kCameraLookahead);
    const auto target_x = nonnegative_sum(entity_x, lookahead.horizontal);
    const auto target_y = nonnegative_sum(entity_y, lookahead.vertical);
    const auto center_x = add_word(state.camera_x, kCameraCenterOffsetX);
    const auto center_y = add_word(state.camera_y, kCameraCenterOffsetY);
    state.horizontal_step = classify_field_camera_axis_step(
        center_x, target_x, state.horizontal_step);
    state.vertical_step = classify_field_camera_axis_step(
        center_y, target_y, state.vertical_step);
    apply_camera_steps(state);
    return state;
}

} // namespace shadowrun::ghidra_conversion
