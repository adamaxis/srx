#include "shadowrun_ghidra/magic_menu_conversion.hpp"

namespace {

bool expect(const bool condition) { return condition; }

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    RunnerRecordFields shaman{};
    shaman.archetype_index = 1U;
    shaman.auxiliary[0] = 6U;
    shaman.magic_table_index = 3U;
    if (!expect(select_magic_menu_path(shaman) == MagicMenuPath::spell_panel)) {
        return 1;
    }

    shaman.auxiliary[0] = 0U;
    if (!expect(select_magic_menu_path(shaman) ==
                    MagicMenuPath::fallback_menu)) {
        return 2;
    }
    shaman.auxiliary[0] = 6U;
    shaman.archetype_index = 2U;
    if (!expect(select_magic_menu_path(shaman) ==
                    MagicMenuPath::fallback_menu)) {
        return 3;
    }

    MagicMenuSelection selection{6U, 0U};
    selection = move_magic_selection_column(selection, 1, 7U);
    if (!expect(selection.column == 0U && selection.row == 0U)) {
        return 4;
    }
    selection = move_magic_selection_row(selection, 1, 7U);
    if (!expect(selection.column == 0U && selection.row == 1U)) {
        return 5;
    }
    selection = move_magic_selection_column({0U, 1U}, -1, 7U);
    if (!expect(selection.column == 0U && selection.row == 1U)) {
        return 6;
    }
    selection = move_magic_selection_row({1U, 1U}, 1, 7U);
    if (!expect(selection.column == 1U && selection.row == 0U)) {
        return 7;
    }
    selection = move_magic_selection_row({1U, 0U}, -1, 7U);
    if (!expect(selection.column == 1U && selection.row == 0U)) {
        return 8;
    }
    selection = move_magic_selection_row({0U, 0U}, 1, 6U);
    if (!expect(selection.column == 0U && selection.row == 0U)) {
        return 9;
    }

    std::vector<std::uint8_t> spell_table{
        0x00U, 0x02U, // two entries
        0x01U, 0x09U, // code 1, source table parameter 9
        0x03U, 0x04U, // code 3, source table parameter 4
    };
    std::vector<std::uint8_t> spell_records(4U * kMagicSpellRecordSize, 0U);
    spell_records[1U * kMagicSpellRecordSize + 0x25U] = 0x02U;
    spell_records[3U * kMagicSpellRecordSize + 0x25U] = 0xFFU;
    const auto available = select_magic_spell(
        spell_table, {0U, 0U}, spell_records);
    if (!expect(available.has_value() && available->table_entry_index == 0U &&
                available->spell_code == 1U &&
                available->table_parameter == 9U &&
                available->availability_byte == 2 &&
                available->availability == MagicSpellAvailability::available)) {
        return 10;
    }
    const auto unavailable = select_magic_spell(
        spell_table, {1U, 0U}, spell_records);
    if (!expect(unavailable.has_value() && unavailable->spell_code == 3U &&
                unavailable->availability_byte == -1 &&
                unavailable->availability ==
                    MagicSpellAvailability::unavailable &&
                !select_magic_spell(spell_table, {2U, 0U}, spell_records)
                     .has_value())) {
        return 11;
    }

    if (!expect(clamp_magic_force(1U, -1, 9U) == 1U &&
                clamp_magic_force(8U, 1, 9U) == 9U &&
                clamp_magic_force(3U, -1, 9U) == 2U)) {
        return 12;
    }
    RunnerRecordFields action_record{};
    action_record.equipped_inventory_slot = 2;
    action_record.equipped_item_code = 7U;
    const auto preparation = prepare_magic_action(
        action_record, *available, 4U);
    if (!expect(preparation.result == MagicActionResult::prepared &&
                preparation.spell_item_code == 2U && preparation.force == 4U &&
                preparation.clear_active_item_marker &&
                action_record.equipped_item_code == 2U &&
                action_record.ammo_rounds == 4U &&
                action_record.magic_force == 4U)) {
        return 13;
    }
    const auto rejected = prepare_magic_action(action_record, *unavailable, 4U);
    if (!expect(rejected.result == MagicActionResult::unavailable_spell &&
                action_record.equipped_item_code == 2U)) {
        return 14;
    }

    MagicForceSelection force_state{1U};
    if (!expect(apply_magic_force_input(force_state, 0x0800U, 3U) ==
                    MagicForceLoopResult::adjusted &&
                force_state.force == 2U &&
                apply_magic_force_input(force_state, 0x0400U, 3U) ==
                    MagicForceLoopResult::adjusted &&
                force_state.force == 1U &&
                apply_magic_force_input(force_state, 0x0400U, 3U) ==
                    MagicForceLoopResult::adjusted &&
                force_state.force == 1U &&
                apply_magic_force_input(force_state, 0x0800U, 1U) ==
                    MagicForceLoopResult::adjusted &&
                force_state.force == 1U &&
                apply_magic_force_input(force_state, 0x0200U, 3U) ==
                    MagicForceLoopResult::finished &&
                apply_magic_force_input(force_state, 0U, 3U) ==
                    MagicForceLoopResult::waiting)) {
        return 15;
    }

    return 0;
}
