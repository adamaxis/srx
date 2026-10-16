#include "shadowrun_ghidra/world/world_effect_controller_conversion.hpp"
#include "shadowrun_ghidra/world/world_effect_overlay_conversion.hpp"

#include <cstdint>
#include <optional>

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectDefaultState world_defaults{};
    initialize_effect_defaults(world_defaults);
    if (world_defaults.animation_mode != 1U ||
        world_defaults.position_x != 0x1200U ||
        world_defaults.position_y != 0x0D00U ||
        world_defaults.frame_countdown != 0x1EU) {
        return 1;
    }
    WorldEffectOverlayState threshold{};
    if (arm_effect_after_threshold(threshold, 0x713FU) ||
        !arm_effect_after_threshold(threshold, 0x7140U) ||
        !threshold.scene_complete || threshold.scene_completion_aux != 0U) {
        return 2;
    }
    WorldEffectResetState reset{true, false, true};
    reset_effect_entity(reset);
    if (reset.descriptor_transition_active || !reset.exit_requested ||
        reset.work_record_active) {
        return 3;
    }

    WorldEffectControllerState inactive{};
    if (advance_world_effect_controller(inactive, {}) !=
        WorldEffectControllerAction::inactive) {
        return 4;
    }

    WorldEffectControllerState descriptor{};
    descriptor.work_record_active = true;
    int descriptor_starts = 0;
    WorldEffectControllerCallbacks descriptor_callbacks{};
    descriptor_callbacks.runner_has_active_effect = [] { return false; };
    descriptor_callbacks.begin_descriptor_transition = [&] {
        ++descriptor_starts;
        descriptor.descriptor_transition_active = true;
    };
    if (advance_world_effect_controller(descriptor, descriptor_callbacks) !=
            WorldEffectControllerAction::descriptor_transition_started ||
        descriptor_starts != 1 || !descriptor.descriptor_transition_active) {
        return 5;
    }

    WorldEffectControllerState loading{};
    loading.work_record_active = true;
    loading.descriptor_transition_active = true;
    loading.descriptor_stream_loading = true;
    if (advance_world_effect_controller(
            loading,
            WorldEffectControllerCallbacks{
                [] { return true; }, {}, {}, {}, {}, {}}) !=
            WorldEffectControllerAction::descriptor_stream_loading) {
        return 6;
    }

    WorldEffectControllerState transition{};
    transition.work_record_active = true;
    transition.descriptor_transition_active = true;
    transition.work.frame_counter = 0U;
    transition.work.descriptor_state = 2U;
    int vblanks = 0;
    std::uint8_t streamed_phase = 0;
    int resets = 0;
    WorldEffectControllerCallbacks transition_callbacks{};
    transition_callbacks.runner_has_active_effect = [] { return true; };
    transition_callbacks.transition.wait_for_vblank = [&] { ++vblanks; };
    transition_callbacks.transition.stream_transition_noise =
        [&](const std::uint8_t phase) { streamed_phase = phase; };
    transition_callbacks.transition.reset_effect_entity = [&] { ++resets; };
    if (advance_world_effect_controller(transition, transition_callbacks) !=
            WorldEffectControllerAction::transition_noise_streamed ||
        vblanks != 1 || streamed_phase != 1U || resets != 0 ||
        transition.work.frame_counter != 2U ||
        transition.work.descriptor_state != 1U) {
        return 7;
    }
    transition.work.frame_counter = 0U;
    transition.work.descriptor_state = 0U;
    if (advance_world_effect_controller(transition, transition_callbacks) !=
            WorldEffectControllerAction::effect_entity_reset ||
        resets != 1 || transition.descriptor_transition_active ||
        transition.work_record_active || !transition.exit_requested) {
        return 8;
    }

    WorldEffectControllerState waiting{};
    waiting.work_record_active = true;
    waiting.world_effects_active = true;
    waiting.animation_mode = 0U;
    waiting.effect_countdown = 2;
    WorldEffectControllerCallbacks waiting_callbacks{};
    waiting_callbacks.runner_has_active_effect = [] { return true; };
    if (advance_world_effect_controller(waiting, waiting_callbacks) !=
            WorldEffectControllerAction::countdown_wait ||
        waiting.effect_countdown != 1) {
        return 9;
    }

    WorldEffectControllerState background{};
    background.work_record_active = true;
    background.world_effects_active = true;
    background.effect_countdown = 0;
    WorldEffectControllerCallbacks background_callbacks{};
    background_callbacks.runner_has_active_effect = [] { return true; };
    background_callbacks.calculate_menu_row_offset =
        [] { return std::optional<std::uint16_t>(4U); };
    background_callbacks.select_effect_mode =
        [] { return std::optional<std::int16_t>(2); };
    if (advance_world_effect_controller(background, background_callbacks) !=
            WorldEffectControllerAction::background_effect_initialized ||
        background.effect_countdown != 4 || background.background_step != 0x280U ||
        background.background_started != 1U ||
        background.background_position != 0x11F0U) {
        return 10;
    }

    WorldEffectControllerState motion{};
    motion.work_record_active = true;
    motion.world_effects_active = true;
    motion.effect_countdown = -1;
    motion.animation_divisor = 2;
    motion.world_effect_mode = 6;
    WorldEffectControllerCallbacks motion_callbacks{};
    motion_callbacks.runner_has_active_effect = [] { return true; };
    motion_callbacks.calculate_menu_row_offset =
        [] { return std::optional<std::uint16_t>(1U); };
    motion_callbacks.select_effect_mode =
        [] { return std::optional<std::int16_t>(3); };
    if (advance_world_effect_controller(motion, motion_callbacks) !=
            WorldEffectControllerAction::motion_effect_initialized ||
        motion.motion_step != 0x0015U || motion.secondary_started != 2U ||
        motion.effect_countdown != 1) {
        return 11;
    }

    WorldEffectControllerState default_controller{};
    default_controller.work_record_active = true;
    default_controller.world_effects_active = true;
    default_controller.effect_countdown = -1;
    int default_initializations = 0;
    WorldEffectControllerCallbacks default_callbacks{};
    default_callbacks.runner_has_active_effect = [] { return true; };
    default_callbacks.calculate_menu_row_offset =
        [] { return std::optional<std::uint16_t>(3U); };
    default_callbacks.select_effect_mode =
        [] { return std::optional<std::int16_t>(1); };
    default_callbacks.initialize_effect_defaults =
        [&] { ++default_initializations; };
    if (advance_world_effect_controller(default_controller, default_callbacks) !=
            WorldEffectControllerAction::default_effect_initialized ||
        default_initializations != 1 ||
        default_controller.animation_mode != 2U ||
        default_controller.title_phase_handler_index != 2U) {
        return 12;
    }

    WorldEffectControllerState missing_runner{};
    missing_runner.work_record_active = true;
    if (advance_world_effect_controller(missing_runner, {}) !=
        WorldEffectControllerAction::runner_effect_status_unresolved) {
        return 13;
    }

    WorldEffectControllerState missing_transition{};
    missing_transition.work_record_active = true;
    WorldEffectControllerCallbacks missing_transition_callbacks{};
    missing_transition_callbacks.runner_has_active_effect = [] {
        return false;
    };
    if (advance_world_effect_controller(missing_transition,
                                        missing_transition_callbacks) !=
        WorldEffectControllerAction::descriptor_transition_unresolved) {
        return 14;
    }

    WorldEffectControllerState zero_background{};
    zero_background.work_record_active = true;
    zero_background.world_effects_active = true;
    zero_background.effect_countdown = -1;
    int zero_calls = 0;
    WorldEffectControllerCallbacks zero_background_callbacks{};
    zero_background_callbacks.runner_has_active_effect = [] { return true; };
    zero_background_callbacks.calculate_menu_row_offset = [&] {
        ++zero_calls;
        return std::optional<std::uint16_t>(0U);
    };
    zero_background_callbacks.select_effect_mode = [] {
        return std::optional<std::int16_t>(2);
    };
    if (advance_world_effect_controller(zero_background,
                                        zero_background_callbacks) !=
            WorldEffectControllerAction::countdown_unresolved ||
        zero_calls != 2 || zero_background.effect_countdown != 0) {
        return 15;
    }

    return 0;
}
