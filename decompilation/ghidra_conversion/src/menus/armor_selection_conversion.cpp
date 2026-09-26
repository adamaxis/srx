#include "shadowrun_ghidra/menus/armor_selection_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

std::uint16_t subtract_modifier(
    const std::uint16_t value,
    const std::uint8_t armor_index,
    const std::array<std::uint8_t, 0x32>& modifiers) {
    if (armor_index == 0U || armor_index > modifiers.size()) {
        return value;
    }
    const auto row = static_cast<std::size_t>(armor_index - 1U);
    const auto low_byte = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(value) - modifiers[row]);
    return static_cast<std::uint16_t>((value & 0xFF00U) | low_byte);
}

} // namespace

std::uint16_t subtract_armor_primary_modifier(
    const std::uint16_t value,
    const std::uint8_t armor_index,
    const ArmorModifierTable& table) {
    return subtract_modifier(value, armor_index, table.primary);
}

std::uint16_t subtract_armor_secondary_modifier(
    const std::uint16_t value,
    const std::uint8_t armor_index,
    const ArmorModifierTable& table) {
    return subtract_modifier(value, armor_index, table.secondary);
}

ArmorSelectionResult apply_armor_selection(
    ArmorSelectionState& state,
    const std::uint8_t selected_item_code,
    const bool preview_new_selection,
    const ArmorSelectionCallbacks& callbacks) {
    if (selected_item_code < 0x2CU) {
        return ArmorSelectionResult::rejected_item_code;
    }

    const auto selected_armor_index = static_cast<std::uint8_t>(
        selected_item_code - 0x2BU);
    const auto previous_armor_index = state.equipped_armor_index;
    const auto previous_phase = state.selected_inventory_phase;

    if (preview_new_selection) {
        state.equipped_armor_index = selected_armor_index;
        state.selected_inventory_phase = selected_armor_index;
    } else if (state.equipped_armor_index == 0U) {
        call_once(callbacks.show_no_equipped_armor);
        return ArmorSelectionResult::rejected_no_equipped_armor;
    }

    // Source order: two text draws, 0x55EB4, one grid write, 0x55E92, and a
    // second grid write. These remain presentation/effect boundaries; the
    // selection mutation and rollback are native typed state.
    call_once(callbacks.render_text);
    call_once(callbacks.render_text);
    call_once(callbacks.refresh_first_effect);
    call_once(callbacks.write_grid);
    call_once(callbacks.refresh_second_effect);
    call_once(callbacks.write_grid);

    if (preview_new_selection) {
        state.equipped_armor_index = previous_armor_index;
        state.selected_inventory_phase = previous_phase;
        return ArmorSelectionResult::previewed_and_restored;
    }
    return ArmorSelectionResult::refreshed_current;
}

} // namespace shadowrun::ghidra_conversion
