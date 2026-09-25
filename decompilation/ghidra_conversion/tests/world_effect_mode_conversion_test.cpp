#include "shadowrun_ghidra/world_effect_mode_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectDescriptorTransitionState loading{};
    loading.descriptor_stream_ready = false;
    auto loading_result = begin_world_effect_descriptor_transition(
        loading, WorldEffectDescriptorTransitionCallbacks{});
    if (loading_result !=
            WorldEffectDescriptorTransitionAction::loading_started ||
        loading.animation_mode != 0U || !loading.transition_active ||
        !loading.descriptor_loading || loading.work.frame_counter != 6U ||
        loading.work.phase_or_descriptor_index != 0U) {
        return 1;
    }

    WorldEffectDescriptorTransitionState attached{};
    attached.descriptor_stream_ready = true;
    attached.work.phase_or_descriptor_index = 3U;
    std::vector<std::uint8_t> requested_indexes;
    bool attached_callback = false;
    WorldEffectDescriptorTransitionCallbacks transition_callbacks{};
    transition_callbacks.read_behavior_token =
        [&](const std::uint8_t index) -> std::optional<std::uint32_t> {
        requested_indexes.push_back(index);
        return 0x12345678U;
    };
    transition_callbacks.attach_behavior_descriptor =
        [&] { attached_callback = true; };
    const auto attached_result = begin_world_effect_descriptor_transition(
        attached, transition_callbacks);
    if (attached_result !=
            WorldEffectDescriptorTransitionAction::descriptor_attached ||
        requested_indexes != std::vector<std::uint8_t>{3U} ||
        !attached.behavior_token.has_value() ||
        *attached.behavior_token != 0x12345678U ||
        attached.behavior_position != 0xD280U ||
        attached.work.frame_counter != 0U ||
        attached.work.descriptor_state != 7U || !attached_callback) {
        return 2;
    }

    WorldEffectDescriptorTransitionState blocked{};
    blocked.completion_gate = true;
    blocked.animation_mode = 9U;
    const auto blocked_result = begin_world_effect_descriptor_transition(
        blocked, WorldEffectDescriptorTransitionCallbacks{});
    if (blocked_result !=
            WorldEffectDescriptorTransitionAction::blocked_by_completion_gate ||
        blocked.animation_mode != 9U || blocked.transition_active) {
        return 3;
    }

    WorldEffectModeState selected{};
    selected.mode_request_pending = true;
    selected.requested_mode = 6;
    bool resolved_mode = false;
    WorldEffectModeCallbacks mode_callbacks{};
    mode_callbacks.resolve_mode_descriptor =
        [&](const std::int16_t mode)
        -> std::optional<WorldEffectModeDescriptor> {
        resolved_mode = mode == 6;
        return WorldEffectModeDescriptor{0x09U, 0x06U, 0x000F1AD6U};
    };
    if (initialize_world_effect_mode(selected, mode_callbacks) !=
            WorldEffectModeAction::initialized ||
        !selected.animation_active || selected.selected_mode != 6 ||
        !selected.mode_six_started || selected.mode_six_position != 0x47C0U ||
        !resolved_mode || selected.work.descriptor_state != 0x09U ||
        selected.work.descriptor_phase != 0x06U ||
        selected.work.descriptor_pointer != 0x000F1AD6U ||
        selected.work.x != 0x0120U || selected.work.y != 0x00D0U) {
        return 4;
    }

    WorldEffectModeState negative{};
    negative.mode_request_pending = true;
    negative.requested_mode = -1;
    if (initialize_world_effect_mode(negative,
                                     WorldEffectModeCallbacks{}) !=
            WorldEffectModeAction::initialized ||
        negative.work.descriptor_state != 0x0CU ||
        negative.work.descriptor_phase != 0x06U ||
        negative.work.descriptor_pointer != 0x0001AA28U) {
        return 5;
    }

    WorldEffectModeState exit{};
    exit.mode_request_pending = true;
    exit.requested_mode = 0x45;
    if (initialize_world_effect_mode(exit, WorldEffectModeCallbacks{}) !=
            WorldEffectModeAction::exit_requested ||
        exit.animation_active || !exit.exit_requested) {
        return 6;
    }

    return 0;
}
