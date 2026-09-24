#include "shadowrun_ghidra/entity_action_state_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void set_entity_action_state(
    EntityActionState& state,
    const std::uint8_t action_code,
    const std::uint8_t default_state_code,
    const std::uint8_t nonzero_state_code,
    const std::function<void()>& service_action,
    const std::function<void()>& prepare_assets) {
    state.action_code = action_code;
    if (action_code == 0U) {
        state.action_code = 0U;
        state.reset_marker = 0U;
        state.state_code = default_state_code;
    } else {
        state.state_code = nonzero_state_code;
        if (state.active_marker != 0U) {
            state.active_marker = 0U;
            state.reset_marker = 0U;
        }
    }
    if (service_action) {
        service_action();
    }
    if (prepare_assets) {
        prepare_assets();
    }
}

void dispatch_entity_action_candidates(
    EntityActionCandidateDispatchState& state,
    const std::function<void()>& dispatch_primary,
    const std::function<void()>& dispatch_secondary) {
    state.service_marker = 0U;
    if (state.alternate_mode) {
        if (dispatch_primary) {
            dispatch_primary();
        }
        if (dispatch_secondary) {
            dispatch_secondary();
        }
        return;
    }

    const auto action_class = static_cast<std::uint8_t>(state.action_flags & 0x3FU);
    if (action_class == 0x0BU) {
        if (dispatch_primary) {
            dispatch_primary();
        }
    } else if (action_class == 0x0FU && dispatch_secondary) {
        dispatch_secondary();
    }
}

void advance_entity_action_threshold(
    const std::function<void()>& find_action,
    const std::function<void()>& set_action_state) {
    if (find_action) {
        find_action();
    }
    if (set_action_state) {
        set_action_state();
    }
}

std::uint8_t select_entity_action_lane(
    const EntityActionLaneAvailability& availability) {
    std::uint8_t lane = 0U;
    if (availability.primary_occupancy > 3U &&
        (availability.secondary_override ||
         availability.secondary_occupancy > 3U)) {
        lane = 1U;
        if (availability.tertiary_override ||
            availability.tertiary_occupancy > 3U) {
            lane = 2U;
        }
    }
    return lane;
}

} // namespace shadowrun::ghidra_conversion
