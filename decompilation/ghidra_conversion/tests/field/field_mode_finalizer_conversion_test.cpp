#include "shadowrun_ghidra/field/field_mode_finalizer_conversion.hpp"

#include <cstdint>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldModeFinalizerState state{};
    FieldModeFinalizerSelection observed{};
    int services = 0;
    int resets = 0;
    int effects = 0;
    std::uint16_t next_random = 74U;
    const FieldModeFinalizerCallbacks callbacks{
        [&](FieldModeFinalizerSelection& selection,
            FieldModeFinalizerState&) {
            observed = selection;
            ++services;
        },
        [&]() { ++resets; },
        [&](const std::uint16_t bound) {
            if (bound != 100U) {
                return static_cast<std::uint16_t>(0U);
            }
            return next_random;
        },
        [&]() { ++effects; }};

    state.scene_id = 6U;
    state.submode = 0U;
    state.mode_flags = (1UL << 9U) | (1UL << 4U);
    if (expect(finalize_field_mode_exit(state, callbacks) ==
                   FieldModeFinalizerAction::handled &&
               (state.mode_flags & (1UL << 9U)) == 0U && services == 1 &&
               state.caller_exit_status == -1) !=
        0) {
        return 1;
    }

    state.scene_id = 7U;
    state.world_flags = 0U;
    if (expect(finalize_field_mode_exit(state, callbacks) ==
                   FieldModeFinalizerAction::handled &&
               (state.world_flags & (1UL << 24U)) != 0U &&
               observed.message_variant ==
                   FieldModeFinalizerMessageVariant::first_visit) != 0) {
        return 2;
    }
    state.world_flags |= 1UL << 25U;
    if (expect(finalize_field_mode_exit(state, callbacks) ==
                   FieldModeFinalizerAction::handled && resets == 1 &&
               observed.message_variant ==
                   FieldModeFinalizerMessageVariant::completed_visit) != 0) {
        return 3;
    }

    state.scene_id = 1U;
    state.submode = 0U;
    state.alternate_result = 0;
    state.caller_exit_status = 0;
    next_random = 74U;
    if (expect(finalize_field_mode_exit(state, callbacks) ==
                   FieldModeFinalizerAction::handled &&
               state.service_status < 0 && effects == 0 &&
               state.caller_exit_status == 0 &&
               observed.message_variant ==
                   FieldModeFinalizerMessageVariant::default_failure_prompt &&
               state.alternate_result == 0) != 0) {
        return 4;
    }
    next_random = 75U;
    if (expect(finalize_field_mode_exit(state, callbacks) ==
                   FieldModeFinalizerAction::handled && effects == 1 &&
               state.caller_exit_status == -1 &&
               observed.message_variant ==
                   FieldModeFinalizerMessageVariant::default_effect_prompt) !=
        0) {
        return 5;
    }

    state.scene_id = 2U;
    state.submode = 7U;
    state.alternate_result = 1;
    if (expect(finalize_field_mode_exit(state, callbacks) ==
                   FieldModeFinalizerAction::handled &&
               observed.message_variant ==
                   FieldModeFinalizerMessageVariant::alternate_profile_positive) !=
        0) {
        return 6;
    }

    state.scene_id = 8U;
    state.submode = 0U;
    if (expect(finalize_field_mode_exit(state, callbacks) ==
                   FieldModeFinalizerAction::no_matching_route &&
               services == 6) != 0) {
        return 7;
    }
    return 0;
}
