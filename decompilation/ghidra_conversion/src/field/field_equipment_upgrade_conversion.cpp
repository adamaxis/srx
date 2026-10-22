#include "shadowrun_ghidra/field/field_equipment_upgrade_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

std::optional<FieldEquipmentUpgradeTarget> resolve_target(
    const FieldEquipmentUpgradeState& state) {
    const auto category = state.selection_category;
    if (category == 2U && (state.upgrade_flags & (1U << 3U)) != 0U) {
        return std::nullopt;
    }

    if (category < 5U) {
        // ROM 0x571F4 sets FFEF96 for Spurs (category 3), replacing the
        // Hand Razors flag at bit 2 after the new flag is set.
        return FieldEquipmentUpgradeTarget{category, category == 3U};
    }

    if (category == 5U) {
        for (std::uint8_t offset = 0; offset < 4U; ++offset) {
            const auto previous_bit = static_cast<std::uint8_t>(5U + offset);
            if ((state.upgrade_flags & (1U << previous_bit)) == 0U) {
                continue;
            }
            if (offset == 3U) {
                return std::nullopt;
            }
            return FieldEquipmentUpgradeTarget{
                static_cast<std::uint8_t>(6U + offset), true};
        }
        return FieldEquipmentUpgradeTarget{5U, false};
    }

    if (category == 6U) {
        for (std::uint8_t offset = 0; offset < 3U; ++offset) {
            const auto previous_bit = static_cast<std::uint8_t>(9U + offset);
            if ((state.upgrade_flags & (1U << previous_bit)) == 0U) {
                continue;
            }
            if (offset == 2U) {
                return std::nullopt;
            }
            return FieldEquipmentUpgradeTarget{
                static_cast<std::uint8_t>(10U + offset), true};
        }
        return FieldEquipmentUpgradeTarget{6U, false};
    }

    const auto base_bit = static_cast<unsigned>(category) + 5U;
    if (base_bit >= 16U) {
        return std::nullopt;
    }
    // ROM 0x5722A scans bits 12..14. Bit 14 is the three-step terminal
    // marker; the loop ends before it can inspect bit 15.
    for (std::uint8_t offset = 0; offset < 3U; ++offset) {
        const auto previous_bit = static_cast<std::uint8_t>(12U + offset);
        if ((state.upgrade_flags & (1U << previous_bit)) == 0U) {
            continue;
        }
        if (offset == 2U) {
            return std::nullopt;
        }
        return FieldEquipmentUpgradeTarget{
            static_cast<std::uint8_t>(base_bit + offset + 1U), true};
    }
    return FieldEquipmentUpgradeTarget{static_cast<std::uint8_t>(base_bit),
                                       false};
}

bool signed_byte_is_positive(const std::uint8_t value) {
    return static_cast<std::int8_t>(value) > 0;
}

} // namespace

std::optional<std::uint8_t> field_equipment_upgrade_display_essence_cost(
    const FieldEquipmentUpgradeTable& table,
    const std::uint8_t selection_category,
    const std::uint16_t upgrade_flags) noexcept {
    if (selection_category >= 8U) {
        return std::nullopt;
    }
    auto threshold_index = static_cast<unsigned>(selection_category);
    if (selection_category >= 6U) {
        threshold_index += 5U;
        if (threshold_index >= 12U &&
            (upgrade_flags & (std::uint16_t{1U} << 12U)) != 0U) {
            ++threshold_index;
        }
    }
    if (threshold_index >= table.meter_thresholds.size()) {
        return std::nullopt;
    }

    auto cost = table.meter_thresholds[threshold_index];
    if (selection_category == 3U &&
        (upgrade_flags & (std::uint16_t{1U} << 2U)) != 0U) {
        --cost;
    }
    return cost;
}

FieldEquipmentUpgradeResult apply_field_equipment_upgrade(
    const FieldEquipmentUpgradeTable& table,
    FieldEquipmentUpgradeState& state,
    const FieldEquipmentUpgradeCallbacks& callbacks) {
    if (callbacks.render_action_grid) {
        callbacks.render_action_grid();
    }

    if (state.selection_category >= 8U) {
        return FieldEquipmentUpgradeResult::invalid_selection;
    }
    if (state.selection_category == 2U &&
        (state.upgrade_flags & (1U << 3U)) != 0U) {
        if (callbacks.show_spurs_make_hand_razors_unnecessary) {
            callbacks.show_spurs_make_hand_razors_unnecessary();
        }
        return FieldEquipmentUpgradeResult::
            spurs_make_hand_razors_unnecessary;
    }

    const auto target = resolve_target(state);
    if (!target || target->bit >= table.meter_thresholds.size() ||
        (state.upgrade_flags & (1U << target->bit)) != 0U) {
        if (callbacks.show_already_owned) {
            callbacks.show_already_owned();
        }
        return FieldEquipmentUpgradeResult::already_owned;
    }

    // 0x572DC checks nuyen before drawing the cost and asking for Yes/No.
    if (state.funds < state.cost) {
        if (callbacks.show_insufficient_funds) {
            callbacks.show_insufficient_funds();
        }
        return FieldEquipmentUpgradeResult::insufficient_funds;
    }
    if (callbacks.confirm_purchase && !callbacks.confirm_purchase()) {
        return FieldEquipmentUpgradeResult::cancelled;
    }

    // 0x57342 applies the chosen essence threshold only after confirmation.
    const auto current_meter = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(state.meter_tens) * 10U +
        static_cast<std::uint16_t>(state.meter_ones));
    auto remaining_meter = static_cast<std::uint8_t>(
        current_meter - table.meter_thresholds[target->bit]);
    if (state.selection_category == 3U &&
        (state.upgrade_flags & (1U << 2U)) != 0U) {
        remaining_meter = static_cast<std::uint8_t>(remaining_meter + 1U);
    }
    if (!signed_byte_is_positive(remaining_meter)) {
        if (callbacks.show_insufficient_essence) {
            callbacks.show_insufficient_essence();
        }
        return FieldEquipmentUpgradeResult::insufficient_essence;
    }

    state.meter_tens = static_cast<std::uint8_t>(remaining_meter / 10U);
    state.meter_ones = static_cast<std::uint8_t>(remaining_meter % 10U);
    state.upgrade_flags = static_cast<std::uint16_t>(
        state.upgrade_flags | (1U << target->bit));
    if (target->replaces_previous && target->bit > 0U) {
        state.upgrade_flags = static_cast<std::uint16_t>(
            state.upgrade_flags & ~(1U << (target->bit - 1U)));
    }
    state.funds -= state.cost;

    if (state.archetype_status < 2) {
        state.saved_meter_tens = state.meter_tens;
        for (auto& tracked_value : state.tracked_meter_values) {
            if (state.meter_tens < tracked_value) {
                tracked_value = state.meter_tens;
            }
        }
    }
    return FieldEquipmentUpgradeResult::applied;
}

} // namespace shadowrun::ghidra_conversion
