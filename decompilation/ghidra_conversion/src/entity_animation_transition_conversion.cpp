#include "shadowrun_ghidra/entity_animation_transition_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

bool is_accepted_event(const std::uint8_t type) {
    return type == 0U || type == 1U || type == 7U || type == 8U;
}

bool is_special_phase(const std::uint8_t phase) {
    return phase == 1U || phase == 2U || phase == 8U || phase == 9U;
}

bool source_rate_carry(
    const EntityAnimationRecordState& state,
    const std::uint8_t event_type,
    const EntityAnimationRateCalculator& calculate_rate) {
    return evaluate_entity_animation_rate(
               state, event_type, calculate_rate)
        .source_carry;
}

bool source_lane_carry(const EntityAnimationLaneSelector& select_lane) {
    return select_lane && select_lane();
}

} // namespace

void collect_entity_animation_events(
    const std::vector<EntityAnimationEvent>& source,
    EntityAnimationEventTable& destination) {
    destination.accepted_events.clear();
    destination.special_argument.reset();
    for (const auto& event : source) {
        if (is_accepted_event(event.type)) {
            destination.accepted_events.push_back(event);
        } else if (event.type == 0x0BU) {
            destination.special_argument = event.argument;
        }
    }
    destination.terminated = true;
}

EntityAnimationRateDecision evaluate_entity_animation_rate(
    const EntityAnimationRecordState& state,
    const std::uint8_t event_type,
    const EntityAnimationRateCalculator& calculate_rate) {
    if (state.rate_threshold == 1U) {
        return {true, 0};
    }
    if (!calculate_rate) {
        // The ROM call is an explicit source boundary. Without a supplied
        // result, leave the candidate blocked rather than inventing a rate.
        return {true, 0};
    }

    const auto residual = calculate_rate(
        static_cast<std::int16_t>(static_cast<std::int8_t>(
            state.event_argument)),
        static_cast<std::int16_t>(static_cast<std::int8_t>(event_type)));
    const auto reached = residual >= 0 &&
                         static_cast<std::uint8_t>(residual) >=
                             state.rate_threshold;
    return {reached, residual};
}

EntityAnimationRateDecision evaluate_entity_animation_rate_from_phase(
    const EntityAnimationRecordState& state,
    const EntityAnimationRateCalculator& calculate_rate) {
    return evaluate_entity_animation_rate(
        state, static_cast<std::uint8_t>(state.phase - 1U), calculate_rate);
}

EntityAnimationTransitionResult run_entity_animation_transition(
    EntityAnimationRecordState& state,
    const EntityAnimationEventTable& events,
    const std::uint8_t inherited_event_type,
    const std::uint8_t inherited_event_argument,
    const EntityAnimationTransitionCallbacks& callbacks) {
    EntityAnimationTransitionResult result{};
    EntityAnimationEvent candidate{inherited_event_type,
                                   inherited_event_argument};
    bool use_accepted_event = false;

    if (events.special_argument.has_value()) {
        result.special_event_considered = true;
        candidate = {0x0BU, *events.special_argument};
        const auto special_rate_reached = source_rate_carry(
            state, candidate.type, callbacks.calculate_rate);
        if (special_rate_reached ||
            !source_lane_carry(callbacks.select_lane)) {
            use_accepted_event = true;
        }
    } else {
        use_accepted_event = true;
    }

    if (use_accepted_event) {
        result.accepted_event_considered = true;
        if (events.accepted_events.empty() || !callbacks.select_event) {
            return result;
        }
        const auto selected_index = callbacks.select_event(
            events.accepted_events.size());
        if (!selected_index.has_value() ||
            *selected_index >= events.accepted_events.size()) {
            return result;
        }
        candidate = events.accepted_events[*selected_index];
        if (source_rate_carry(
                state, candidate.type, callbacks.calculate_rate)) {
            return result;
        }
    }

    if (state.phase != 0U && state.phase < 0x0DU &&
        callbacks.clear_auxiliary_marker) {
        const auto replacement = callbacks.clear_auxiliary_marker();
        if (replacement.has_value()) {
            state.auxiliary_marker = *replacement;
        }
    }
    state.frame = static_cast<std::int8_t>(-1);
    state.phase = static_cast<std::uint8_t>(candidate.type + 1U);
    state.event_argument = candidate.argument;
    if (callbacks.reset_delay) {
        callbacks.reset_delay(0U);
    }
    if (callbacks.refresh_assets) {
        callbacks.refresh_assets();
    }

    result.source_carry = false;
    result.state_published = true;
    result.selected_event = candidate;
    return result;
}

bool check_entity_animation_transition(
    const EntityAnimationRecordState& state,
    const EntityAnimationEventTable& events,
    const EntityAnimationLaneSelector& select_lane) {
    if (is_special_phase(state.phase)) {
        if (!events.special_argument.has_value()) {
            return true;
        }
        return !source_lane_carry(select_lane);
    }
    if (state.phase == 0x0CU) {
        return source_lane_carry(select_lane);
    }
    return false;
}

EntityAnimationBranchResult run_entity_animation_branch(
    EntityAnimationRecordState& state,
    const EntityAnimationEventTable& events,
    const std::uint8_t inherited_event_type,
    const std::uint8_t inherited_event_argument,
    const EntityAnimationBranchCallbacks& callbacks) {
    EntityAnimationBranchResult result{};
    if (static_cast<std::int8_t>(state.branch_mode) >= 2) {
        return result;
    }
    result.entered = true;

    if (state.frame < 0 &&
        check_entity_animation_transition(
            state, events, callbacks.transition.select_lane)) {
        result.rate_checked = true;
        const auto rate = evaluate_entity_animation_rate_from_phase(
            state, callbacks.transition.calculate_rate);
        if (rate.source_carry) {
            if (callbacks.advance_action_threshold) {
                callbacks.advance_action_threshold();
            }
            result.action_threshold_advanced = true;
            return result;
        }
        return result;
    }

    if (static_cast<std::int8_t>(state.rate_threshold) < 5) {
        return result;
    }

    result.transition_called = true;
    const auto transition = run_entity_animation_transition(
        state, events, inherited_event_type, inherited_event_argument,
        callbacks.transition);
    result.transition_published = transition.state_published;
    if (transition.source_carry) {
        if (callbacks.advance_action_threshold) {
            callbacks.advance_action_threshold();
        }
        result.action_threshold_advanced = true;
    }
    return result;
}

EntityAnimationEntryResult run_entity_animation_entry(
    EntityAnimationEntryState& state,
    const EntityAnimationEntryCallbacks& callbacks) {
    EntityAnimationEntryResult result{};
    state.timer = static_cast<std::uint16_t>(state.timer - 1U);
    if (static_cast<std::int16_t>(state.timer) >= 0) {
        return result;
    }
    result.timer_expired = true;

    if (state.global_frame_lock) {
        result.action = EntityAnimationEntryAction::suppressed_by_frame_lock;
        return result;
    }

    state.global_frame_lock = true;
    state.timer = 0x0014U;
    result.timer_rearmed = true;

    if (!state.record_animation_enabled ||
        !state.global_animation_enabled) {
        if (callbacks.fallback_update) {
            callbacks.fallback_update();
        }
        result.action = EntityAnimationEntryAction::fallback_update;
        return result;
    }

    if (state.transition_marker < 0) {
        if (callbacks.animation_transition) {
            callbacks.animation_transition();
        }
        result.transition_requested = true;
    }
    if (callbacks.continue_entity_update) {
        callbacks.continue_entity_update();
    }
    result.action = EntityAnimationEntryAction::continue_entity_update;
    return result;
}

} // namespace shadowrun::ghidra_conversion
