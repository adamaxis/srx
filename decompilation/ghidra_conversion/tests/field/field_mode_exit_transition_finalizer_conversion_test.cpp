#include "shadowrun_ghidra/field/field_mode_exit_transition_finalizer_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldModeExitTransitionState state{};
    state.exit_mode = 5U;
    state.transition_flags = 0x01U;
    state.world_mode_flags = 0x01U;
    state.effect_service_flags = 0xFFFFU;
    state.runner_flags = 0xFFU;
    state.recruited_runner_flags = 0xFFU;
    state.action_counter = 5U;
    state.resource_counter = 7U;
    state.resource_pool = 60000U;

    std::vector<int> events;
    std::optional<ExitEffectSlotGroup> cleared_group;
    std::vector<FieldModeExitServicePhase> service_phases;
    bool reset_callback_saw_cleared_rom_state = false;
    const auto result = finalize_field_mode_exit_transition(
        state,
        {[&](const ExitEffectSlotGroup group) {
             cleared_group = group;
             events.push_back(1);
         },
         [&] {
             reset_callback_saw_cleared_rom_state =
                 state.exit_mode == 0xFFU && state.transition_flags == 0U;
             events.push_back(2);
         },
         [&](const FieldModeExitServicePhase phase) {
             service_phases.push_back(phase);
             events.push_back(phase == FieldModeExitServicePhase::mode_specific_cleanup
                                  ? 3
                                  : 7);
         },
         [&] { events.push_back(4); },
         [&] { events.push_back(5); },
         [&] { events.push_back(6); },
         [&] { events.push_back(8); },
         [&] { events.push_back(9); },
         [&] { events.push_back(10); }});

    if (!result.mode_specific_cleanup_ran ||
        !result.cleared_effect_group.has_value() ||
        *result.cleared_effect_group != ExitEffectSlotGroup::mode_five ||
        cleared_group != result.cleared_effect_group ||
        result.initialized_profile != FieldModeProfile::profile_a ||
        events != std::vector<int>{10, 1, 2, 3, 4, 7, 5, 6, 8} ||
        service_phases != std::vector<FieldModeExitServicePhase>{
                               FieldModeExitServicePhase::mode_specific_cleanup,
                               FieldModeExitServicePhase::common_state_refresh} ||
        !reset_callback_saw_cleared_rom_state || state.exit_mode != 0xFFU ||
        state.effect_transition_marker != 0U ||
        state.effect_animation_marker != 0U || state.mode_prompt_pending != 0U ||
        state.mode_five_effect_marker != 0U ||
        state.transition_flags != 0U ||
        state.primary_runner_transition != 0U ||
        state.secondary_runner_transition != 0U || state.runner_flags != 0xAFU ||
        state.recruited_runner_flags != 0xBFU || state.field_anchor != 0xB45CU ||
        state.action_counter != 6U || state.resource_counter != 8U ||
        state.resource_pool != 55000U) {
        return 1;
    }

    FieldModeExitTransitionState mode_two{};
    mode_two.exit_mode = 2U;
    mode_two.transition_flags = 0x01U;
    mode_two.effect_service_flags = 1UL << 9U | 0x05U;
    const auto mode_two_result = finalize_field_mode_exit_transition(
        mode_two,
        {[&](const ExitEffectSlotGroup group) {
             if (group != ExitEffectSlotGroup::mode_two_or_four) {
                 events.push_back(99);
             }
         },
         {},
         {},
         {},
         {},
         {},
         {}});
    if (!mode_two_result.mode_specific_cleanup_ran ||
        mode_two.exit_mode != 0xFFU || mode_two.transition_flags != 0U ||
        (mode_two.effect_service_flags & (1UL << 9U)) != 0U ||
        (mode_two.effect_service_flags & 0x05U) != 0x05U) {
        return 2;
    }

    FieldModeExitTransitionState rejected{};
    rejected.exit_mode = 5U;
    rejected.transition_flags = 0x81U;
    bool cleared = false;
    bool conditional_actor_choice_cleared = false;
    int common_services = 0;
    const auto rejected_result = finalize_field_mode_exit_transition(
        rejected,
        {[&](const ExitEffectSlotGroup) { cleared = true; },
         {},
         [&](const FieldModeExitServicePhase phase) {
             if (phase == FieldModeExitServicePhase::common_state_refresh) {
                 ++common_services;
             }
         },
         {},
         {},
         {},
         {},
         {},
         [&] { conditional_actor_choice_cleared = true; }});
    if (rejected_result.mode_specific_cleanup_ran || cleared ||
        conditional_actor_choice_cleared || rejected.transition_flags != 0x81U ||
        rejected.exit_mode != 5U ||
        common_services != 1) {
        return 3;
    }

    return 0;
}
