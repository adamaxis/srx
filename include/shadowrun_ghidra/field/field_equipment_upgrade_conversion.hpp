#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct FieldEquipmentUpgradeTable {
    // ROM 0x1AA8A0, indexed by the resolved FFF0D5 target bit.
    std::array<std::uint8_t, 16> meter_thresholds{};
};

struct FieldEquipmentUpgradeState {
    std::uint16_t upgrade_flags = 0; // active runner +0x90
    std::uint8_t selection_category = 0; // FFF0D0
    std::uint32_t cost = 0; // FFF0BE
    std::uint32_t funds = 0; // FFFBFE
    std::uint8_t meter_tens = 0; // active runner +0x7E
    std::uint8_t meter_ones = 0; // active runner +0x8E
    std::uint8_t saved_meter_tens = 0; // active runner +0x7F
    std::int8_t archetype_status = 0; // active runner +0x9B
    std::vector<std::uint8_t> tracked_meter_values; // FFF10E entries
};

struct FieldEquipmentUpgradeTarget {
    std::uint8_t bit = 0;
    bool replaces_previous = false; // FFEF96
};

enum class FieldEquipmentUpgradeResult {
    applied,
    unavailable,
    invalid_selection,
    insufficient_funds,
    cancelled,
};

struct FieldEquipmentUpgradeCallbacks {
    // 0x57C46 is drawn before the branch tests. Presentation messages and
    // confirmation remain explicit boundaries rather than hidden state.
    std::function<void()> render_action_grid;
    std::function<void()> show_unavailable;
    std::function<void()> show_insufficient_funds;
    std::function<bool()> confirm_purchase;
};

// ROM 0x571B2, the menu-result-1 equipment/action upgrade branch. Resolve the
// category's target bit, apply the source conflict/threshold checks, charge
// the confirmed cost, and update the runner's upgrade flags and meter digits.
FieldEquipmentUpgradeResult apply_field_equipment_upgrade(
    const FieldEquipmentUpgradeTable& table,
    FieldEquipmentUpgradeState& state,
    const FieldEquipmentUpgradeCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
