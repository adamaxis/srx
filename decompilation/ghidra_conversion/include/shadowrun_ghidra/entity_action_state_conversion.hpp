#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct EntityActionState {
    std::uint8_t action_code = 0;
    std::uint8_t state_code = 0;
    std::uint8_t active_marker = 0;
    std::uint8_t reset_marker = 0;
};

// ROM 0x13466: publish an action/state pair, clear the source reset markers
// on the zero/nonzero branches, then refresh action assets.
void set_entity_action_state(
    EntityActionState& state,
    std::uint8_t action_code,
    std::uint8_t default_state_code,
    std::uint8_t nonzero_state_code,
    const std::function<void()>& service_action,
    const std::function<void()>& prepare_assets);

struct EntityActionCandidateDispatchState {
    std::uint8_t service_marker = 0;
    std::uint8_t action_flags = 0;
    bool alternate_mode = false;
};

// ROM 0x13874: clear the service marker and dispatch candidate collection
// according to the source mode/low-six-bit action flags.
void dispatch_entity_action_candidates(
    EntityActionCandidateDispatchState& state,
    const std::function<void()>& dispatch_primary,
    const std::function<void()>& dispatch_secondary);

// ROM 0x13AE6: find the first action below the source threshold, then publish
// the resulting entity action state.
void advance_entity_action_threshold(
    const std::function<void()>& find_action,
    const std::function<void()>& set_action_state);

} // namespace shadowrun::ghidra_conversion
