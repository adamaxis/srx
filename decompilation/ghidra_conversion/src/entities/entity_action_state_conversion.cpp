#include "shadowrun_ghidra/entities/entity_action_state_conversion.hpp"
#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void set_entity_action_state(
    EntityActionState& state,
    const std::uint8_t action_code,
    const std::uint8_t nonzero_state_code,
    const std::function<void()>& service_action,
    const std::function<void()>& prepare_assets) {
    state.action_code = action_code;
    state.scaled_ammo = 0U;
    if (action_code == 0U) {
        state.action_code = 0U;
        state.reset_marker = 0U;
        state.state_code = kUnequippedActionType;
    } else {
        state.state_code = nonzero_state_code;
        if (state.active_marker != 0U) {
            const auto row = static_cast<std::uint8_t>(action_code - 1U);
            const auto& catalog = us_item_catalog();
            const auto factor =
                row < catalog.size() ? catalog[row].source_combat_factor
                                     : std::uint8_t{0};
            const auto product =
                static_cast<std::uint32_t>(state.active_marker) * factor;
            auto quotient = static_cast<std::uint16_t>(product / 100U);
            const auto remainder = static_cast<std::uint16_t>(product % 100U);
            if (remainder >= 0x32U) {
                quotient = static_cast<std::uint16_t>(quotient + 1U);
            }
            state.scaled_ammo = quotient;
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
    EntityActionState& state,
    const std::array<std::uint8_t, 8>& action_values,
    const std::function<void()>& service_action,
    const std::function<void()>& prepare_assets) {
    const auto scan = scan_entity_action_threshold(action_values);
    set_entity_action_state(state, scan.action_code, scan.skipped,
                            service_action, prepare_assets);
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

void collect_entity_action_records(
    const std::vector<EntityActionSourceRecord>& source,
    EntityActionRecordTable& destination) {
    destination.selected.clear();
    destination.special_value = 0U;
    for (const auto& record : source) {
        switch (record.code) {
        case 0U:
        case 1U:
        case 7U:
        case 8U:
            destination.selected.push_back(record);
            break;
        case 11U:
            destination.special_value = record.value;
            break;
        default:
            break;
        }
    }
    destination.terminated = true;
}

} // namespace shadowrun::ghidra_conversion
