#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class FieldModeFinalizerResourceProfile {
    scene_zero_submode_one,
    scene_zero_submode_two,
    scene_zero_submode_four,
    scene_zero_submode_five,
    scene_zero_submode_six,
    scene_zero_submode_seven,
    scene_zero_submode_eight_or_nine,
    scene_one_submode_zero,
    scene_one_submode_two,
    scene_one_submode_four,
    scene_one_submode_eight_or_ten,
    scene_one_submode_nine,
    scene_two_submode_zero,
    scene_two_submode_three,
    scene_two_submode_four,
    scene_two_submode_five,
    scene_two_submode_seven,
    scene_two_submode_eight_or_ten,
    scene_two_submode_nine,
    scene_three_submode_zero,
    scene_three_submode_two,
    scene_three_submode_three,
    scene_three_submode_four,
    scene_three_submode_five,
    scene_three_submode_six,
    scene_three_submode_seven_or_eight_or_nine,
    scene_four_submode_one,
    scene_four_submode_two,
    scene_four_submode_five,
    scene_five_submode_one,
    scene_five_submode_six,
    scene_six_submode_zero,
    scene_seven_submode_zero,
    scene_nine_submode_zero,
};

enum class FieldModeFinalizerMessageVariant {
    default_prompt,
    positive_result_prompt,
    negative_result_prompt,
    default_effect_prompt,
    default_failure_prompt,
    first_visit,
    pending_visit,
    completed_visit,
    fallback_visit,
    alternate_profile_positive,
};

struct FieldModeFinalizerState {
    std::uint8_t scene_id = 0;
    std::uint8_t submode = 0;
    std::int8_t alternate_result = 0; // FFF0D5
    std::int8_t service_status = 0; // FFF0D6
    std::uint32_t world_flags = 0; // FFFC1E
    std::uint32_t status_flags = 0; // FFFC22
    std::uint32_t mode_flags = 0; // FFFC30
    std::int16_t caller_exit_status = 0; // D6
};

struct FieldModeFinalizerSelection {
    FieldModeFinalizerResourceProfile resource_profile =
        FieldModeFinalizerResourceProfile::scene_zero_submode_one;
    FieldModeFinalizerMessageVariant message_variant =
        FieldModeFinalizerMessageVariant::default_prompt;
};

struct FieldModeFinalizerCallbacks {
    std::function<void(FieldModeFinalizerSelection&, FieldModeFinalizerState&)>
        run_mode_exit_services;
    std::function<void()> reset_runner_status_triplet;
    std::function<std::uint16_t(std::uint16_t)> sample_source_random_bound;
    std::function<void()> forward_effect_sequence;
};

enum class FieldModeFinalizerAction {
    no_matching_route,
    handled,
};

// ROM 0x1FD80: handle the remaining scene/submode mode-exit routes after the
// earlier field-exit selectors miss. The native conversion publishes a typed
// resource/message choice, keeps FFF0D6 separate from caller D6, and leaves
// the host service boundary injectable.
FieldModeFinalizerAction finalize_field_mode_exit(
    FieldModeFinalizerState& state,
    const FieldModeFinalizerCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
