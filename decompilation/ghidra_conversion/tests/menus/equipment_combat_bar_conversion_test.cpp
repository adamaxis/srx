#include "shadowrun_ghidra/menus/equipment_combat_bar_conversion.hpp"
#include "shadowrun_ghidra/menus/magic_menu_conversion.hpp"
#include "shadowrun_ghidra/core/asset_catalog_conversion.hpp"
#include "shadowrun_ghidra/session/runner_record_access_conversion.hpp"
#include "shadowrun_ghidra/session/startup_record_assets_conversion.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <filesystem>
#include <iostream>
#include <vector>

namespace {

struct RomProfileBarExpectation {
    std::uint8_t race = 0;
    std::uint8_t runner_class = 0;
    std::uint8_t command_average = 0;
    std::array<std::uint8_t, 5U> attack_values{};
    std::array<std::uint8_t, 5U> defense_values{};
};

constexpr std::array<RomProfileBarExpectation, 13U> kRomProfileBars = {{
    {0U, 2U, 4U, {4U, 5U, 6U, 7U, 8U}, {8U, 7U, 6U, 5U, 4U}},
    {0U, 3U, 6U, {2U, 3U, 4U, 4U, 4U}, {8U, 7U, 5U, 4U, 2U}},
    {0U, 1U, 5U, {2U, 3U, 4U, 4U, 4U}, {7U, 6U, 5U, 4U, 2U}},
    {2U, 1U, 4U, {2U, 3U, 4U, 4U, 4U}, {8U, 7U, 6U, 5U, 4U}},
    {4U, 2U, 3U, {4U, 4U, 5U, 6U, 7U}, {10U, 10U, 9U, 8U, 7U}},
    {0U, 0U, 5U, {2U, 3U, 4U, 4U, 4U}, {7U, 6U, 5U, 4U, 2U}},
    {2U, 3U, 5U, {4U, 5U, 6U, 7U, 8U}, {9U, 8U, 7U, 6U, 4U}},
    {3U, 1U, 5U, {3U, 4U, 5U, 6U, 6U}, {11U, 10U, 9U, 8U, 6U}},
    {1U, 3U, 7U, {4U, 5U, 7U, 8U, 8U}, {10U, 9U, 7U, 5U, 3U}},
    {0U, 2U, 7U, {6U, 7U, 9U, 11U, 12U}, {12U, 11U, 9U, 7U, 5U}},
    {1U, 0U, 7U, {6U, 7U, 9U, 11U, 12U}, {10U, 9U, 7U, 5U, 3U}},
    {1U, 3U, 9U, {4U, 6U, 8U, 8U, 8U}, {10U, 8U, 6U, 4U, 1U}},
    {0U, 2U, 8U, {6U, 8U, 10U, 12U, 12U}, {16U, 14U, 12U, 10U, 8U}},
}};

constexpr std::array<std::array<std::uint16_t, 3U>, 3U>
    kRomTotemActionModifiers = {{{0x0002U, 0xFFFFU, 0x0000U},
                                 {0xFFFFU, 0x0002U, 0x0000U},
                                 {0x0000U, 0x0000U, 0x0002U}}};

shadowrun::ghidra_conversion::EquipmentCombatInput make_profile_bar_input(
    const shadowrun::ghidra_conversion::RunnerRecordFields& record) {
    using namespace shadowrun::ghidra_conversion;
    EquipmentCombatInput input{};
    input.attributes = record.attributes;
    input.skills = {record.skills[1],  record.skills[2], record.skills[3],
                    record.skills[4],  record.skills[5], record.auxiliary[1],
                    record.skills[6],  record.skills[7], record.skills[8],
                    record.skills[9],  record.skills[10], record.skills[11]};
    input.inventory_item_codes = record.inventory_item_codes;
    input.inventory_phase_bytes = record.inventory_phase_bytes;
    input.display_state = record.equipped_inventory_slot;
    input.active_item_code = record.equipped_item_code;
    input.posture = record.posture;
    input.selected_inventory_slot =
        static_cast<std::uint8_t>(record.inventory_item_codes.size());
    input.cyberware_flags = record.cyberware_flags;
    input.action_flags = static_cast<std::uint8_t>(record.flags & 0xFFU);
    input.source_mode = 2U;
    return input;
}

bool check_rom_profile_bars(
    const shadowrun::ghidra_conversion::LiveRunnerRecord& live_record,
    const RomProfileBarExpectation& expected,
    const std::size_t profile_index) {
    using namespace shadowrun::ghidra_conversion;
    const auto fields = read_runner_record_fields(live_record);
    if (fields.species_index != expected.race ||
        fields.archetype_index != expected.runner_class) {
        std::cerr << "ROM combat-bar profile identity mismatch at row "
                  << profile_index << '\n';
        return false;
    }
    auto input = make_profile_bar_input(fields);
    for (std::uint8_t posture = 0U; posture <= 4U; ++posture) {
        input.posture = posture;
        const auto bars = compute_equipment_combat_bars(input);
        const auto attack = expected.attack_values[posture];
        const auto defense = expected.defense_values[posture];
        if (!bars.attack_written || !bars.defense_written ||
            bars.command_average != expected.command_average ||
            bars.attack_value != attack || bars.attack_bar != attack - 1U ||
            bars.defense_value != defense ||
            bars.defense_bar != defense - 1U) {
            std::cerr << "ROM combat-bar profile mismatch at row "
                      << profile_index << " posture="
                      << static_cast<unsigned>(posture)
                      << " average=" << bars.command_average
                      << " attack=" << bars.attack_value << "/"
                      << static_cast<unsigned>(bars.attack_bar)
                      << " defense=" << bars.defense_value << "/"
                      << static_cast<unsigned>(bars.defense_bar) << '\n';
            return false;
        }
    }
    return true;
}

std::uint16_t add_source_byte_to_word(const std::uint16_t value,
                                     const std::uint8_t addition) {
    const auto low = static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(value) + addition);
    return static_cast<std::uint16_t>((value & 0xFF00U) | low);
}

std::uint16_t expected_rom_spell_attack_value(
    const shadowrun::ghidra_conversion::LiveRunnerRecord& record,
    const std::uint8_t spell_code,
    const std::uint8_t force,
    const std::uint8_t posture) {
    using namespace shadowrun::ghidra_conversion;

    // This test oracle follows the source field operations at 0x12FA4 and
    // 0x12CEE independently of compute_equipment_combat_bars: item 0x18,
    // posture reduction, the +0x7F cap, +0x9D force, matching spell focus,
    // and the C8AE0/D4540 slot-7 totem modifier.
    std::uint16_t sorcery = record.bytes[0x80U];
    for (std::size_t slot = 7U; slot-- > 0U;) {
        if (record.bytes[0x66U + slot] != 0x18U) {
            continue;
        }
        sorcery = add_source_byte_to_word(
            sorcery, record.bytes[0x6EU + slot]);
        break;
    }
    const auto posture_reduction = static_cast<std::uint16_t>(
        (static_cast<std::uint32_t>(sorcery) * posture * 25U) / 100U);
    auto value = static_cast<std::uint8_t>(std::min<std::uint16_t>(
        posture_reduction, record.bytes[0x7FU]));
    value = static_cast<std::uint8_t>(value + force);

    const auto matching_spell_focus =
        static_cast<std::uint8_t>(0x19U + spell_code);
    for (std::size_t slot = 7U; slot-- > 0U;) {
        if (record.bytes[0x66U + slot] != matching_spell_focus) {
            continue;
        }
        value = static_cast<std::uint8_t>(
            value + record.bytes[0x6EU + slot]);
        break;
    }

    const auto totem_code = record.bytes[0x6DU];
    if (totem_code >= 0x29U && totem_code <= 0x2BU) {
        const auto action_index = static_cast<std::size_t>(spell_code);
        const auto selector =
            us_special_item_modifier_selector_table()
                .field_selectors[action_index];
        if (selector >= 0 && selector <= 2) {
            const auto totem_index =
                static_cast<std::size_t>(totem_code - 0x29U);
            const auto modifier = kRomTotemActionModifiers[totem_index]
                                                               [static_cast<std::size_t>(selector)];
            value = static_cast<std::uint16_t>(
                static_cast<std::uint16_t>(value) +
                static_cast<std::int16_t>(modifier));
            if (static_cast<std::int16_t>(value) <= 0) {
                value = 1U;
            }
        }
    }
    return value;
}

shadowrun::ghidra_conversion::EquipmentCombatInput make_spell_bar_input(
    const shadowrun::ghidra_conversion::LiveRunnerRecord& live_record,
    const shadowrun::ghidra_conversion::MagicSpellDefinition& spell,
    const std::uint8_t force,
    const std::uint8_t posture) {
    using namespace shadowrun::ghidra_conversion;
    const auto fields = read_runner_record_fields(live_record);
    auto input = make_profile_bar_input(fields);
    input.display_state = -1;
    input.active_item_code =
        static_cast<std::uint8_t>(spell.spell_code + 1U);
    input.source_action_code = input.active_item_code;
    input.posture = posture;
    input.special_item18_base = live_record.bytes[0x80U];
    input.roll_count_cap = live_record.bytes[0x7FU];
    input.auxiliary_roll_bonus = force;
    input.special_magic_spell = spell;
    input.item_catalog = &us_item_catalog();
    input.selector_table = &us_special_item_modifier_selector_table();
    return input;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    const auto action_skill = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{4U, 7U, 1U, 3U, 8U, 2U});
    const auto action_skill_next_band = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{4U, 8U, 1U, 3U, 8U, 2U});
    const auto action_skill_final_band = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{4U, 11U, 1U, 3U, 8U, 2U});
    const auto action_skill_zero = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{0U, 0U, 0U, 0U, 0U, 0U});
    if (action_skill.selected_skill != 3U || action_skill.value != 6U ||
        action_skill_next_band.selected_skill != 8U ||
        action_skill_next_band.value != 12U ||
        action_skill_final_band.selected_skill != 2U ||
        action_skill_final_band.value != 4U || action_skill_zero.value != 1U) {
        return 100;
    }

    // 0x12C74 uses wrapped CMP.B predicates. 0xFF falls through the first
    // action band, and 0x01 - 0x80 has its byte sign bit set, selecting 0x80.
    const auto wrapped_action = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{4U, 0xFFU, 1U, 2U, 3U, 4U});
    const auto wrapped_skill = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{1U, 0U, 1U, 0x80U, 3U, 4U});
    const auto wrapped_word = merge_equipment_action_skill_value(
        EquipmentActionSkillMergeInput{0xFFFFU, 0x0BU, 0xFFU, 0U, 0U, 0xFFU});
    if (wrapped_action.selected_skill != 2U || wrapped_action.value != 4U ||
        wrapped_skill.selected_skill != 0x80U || wrapped_skill.value != 0x81U ||
        wrapped_word.value != 0x00FEU) {
        return 101;
    }

    EquipmentCombatInput input{};
    input.attributes = {2U, 4U, 0U, 0U, 4U, 4U, 0U, 0U};
    input.skills[1] = 2U;
    input.skills[2] = 3U;
    input.inventory_item_codes[0] = 0x28U;
    input.inventory_phase_bytes[0] = 2U;
    input.inventory_item_codes[6] = 2U;
    input.display_state = 0;
    input.active_item_code = 2U;
    input.posture = 2U;
    input.selected_inventory_slot = 6U;

    const auto result = compute_equipment_combat_bars(input);
    if (result.command_average != 8U || result.attack_value != 4U ||
        result.attack_bar != 3U || result.defense_value != 6U ||
        result.defense_bar != 5U || !result.attack_written ||
        !result.defense_written) {
        return 1;
    }

    input.source_mode = 2U;
    input.display_state = 0x7F;
    input.skills[4] = 4U;
    const auto unarmed = compute_equipment_combat_bars(input);
    if (unarmed.attack_value != 8U || unarmed.attack_bar != 7U) {
        return 2;
    }

    input.inventory_item_codes[6] = 0x10U;
    const auto utility_selected = compute_equipment_combat_bars(input);
    if (!utility_selected.defense_written ||
        utility_selected.attack_bar != unarmed.attack_bar) {
        return 3;
    }

    input.source_mode = 0U;
    input.source_action_code = 2U;
    input.display_state = -1;
    const auto special = compute_equipment_combat_bars(input);
    if (!special.attack_written || !special.defense_written ||
        special.special_action.menu_service_called) {
        return 4;
    }

    input.source_mode = 2U;
    input.roll_count_cap = 12U;
    input.auxiliary_roll_bonus = 3U;
    input.special_item18_base = 20U;
    input.inventory_item_codes[0] = 0x18U;
    input.inventory_phase_bytes[0] = 2U;
    input.inventory_item_codes[1] = 0x1EU;
    input.inventory_phase_bytes[1] = 4U;
    input.inventory_item_codes[6] = 0x10U;
    input.active_item_code = 6U;
    input.special_magic_spell = *find_us_magic_spell(5U);
    EntityActionSpecialItemModifierTable special_item_modifiers{};
    special_item_modifiers.action_offsets[5] = 1;
    special_item_modifiers.slot7_item_modifiers[0][1] = -2;
    input.inventory_item_codes[7] = 0x29U;
    input.special_item_modifier_table = special_item_modifiers;
    const auto special_mode = compute_equipment_combat_bars(input);
    if (!special_mode.attack_written || !special_mode.defense_written ||
        special_mode.attack_value != 16U || special_mode.attack_bar != 15U ||
        !special_mode.special_action.menu_service_called ||
        !special_mode.special_action.residual_calculated ||
        special_mode.special_action.effect_written ||
        special_mode.special_action.adjusted_roll_count != 7U ||
        special_mode.special_action.effective_difficulty != 2U ||
        special_mode.special_action.residual_effect != -1 ||
        special_mode.special_action.magic_spell_missing) {
        return 5;
    }

    // Source-bound equipment bars use item 41's actual D4540 +0x22 word
    // through C8AE0 action index 5, rather than the caller-owned -2 matrix.
    auto source_bound_input = input;
    source_bound_input.item_catalog = &us_item_catalog();
    source_bound_input.selector_table =
        &us_special_item_modifier_selector_table();
    const auto source_bound_mode =
        compute_equipment_combat_bars(source_bound_input);
    if (!source_bound_mode.attack_written ||
        source_bound_mode.source_data_missing ||
        source_bound_mode.attack_value != 20U ||
        source_bound_mode.attack_bar != 19U) {
        return 30;
    }

    auto incomplete_source_input = source_bound_input;
    incomplete_source_input.selector_table = nullptr;
    const auto incomplete_source_mode =
        compute_equipment_combat_bars(incomplete_source_input);
    if (!incomplete_source_mode.source_data_missing ||
        incomplete_source_mode.attack_value != 18U) {
        return 31;
    }

    input.special_magic_spell->spell_code = 4U;
    const auto mismatched_special = compute_equipment_combat_bars(input);
    if (!mismatched_special.special_action.magic_spell_missing ||
        mismatched_special.special_action.residual_calculated) {
        return 10;
    }

    input.display_state = 0;
    input.active_item_code = 0x10U;
    const auto unsupported_action = compute_equipment_combat_bars(input);
    if (!unsupported_action.attack_written ||
        unsupported_action.attack_value != 1U ||
        unsupported_action.attack_bar != 0U ||
        !unsupported_action.defense_written) {
        return 6;
    }

    input.source_mode = 0U;
    input.action_flags = 0x20U;
    input.inventory_item_codes[0] = 0U;
    input.inventory_phase_bytes[0] = 0U;
    input.selected_inventory_slot = 2U;
    input.active_item_code = 2U;
    const auto flagged_defense = compute_equipment_combat_bars(input);
    if (flagged_defense.defense_value != 3U) {
        return 7;
    }

    // ROM 0x12E52..0x12E6A awards only the first set bit in +0x90 bits
    // 9..11; an empty group adds nothing. Sweep all eight bit combinations.
    EquipmentCombatInput defense_source_case{};
    defense_source_case.attributes = {5U, 6U, 0U, 0U, 6U, 6U, 0U, 0U};
    defense_source_case.source_mode = 2U;
    defense_source_case.display_state = 0x7F;
    defense_source_case.skills[4] = 2U;
    const std::array<std::uint16_t, 8U> expected_cyberware_defense = {
        14U, 15U, 16U, 15U, 17U, 15U, 16U, 15U};
    for (std::uint16_t mask = 0U; mask < expected_cyberware_defense.size();
         ++mask) {
        defense_source_case.cyberware_flags =
            static_cast<std::uint16_t>(mask << 9U);
        const auto cyberware_result =
            compute_equipment_combat_bars(defense_source_case);
        if (!cyberware_result.defense_written ||
            cyberware_result.command_average != 9U ||
            cyberware_result.defense_value !=
                expected_cyberware_defense[mask]) {
            return 32;
        }
    }

    // The first item-0x27 slot is used, while +0x11 bit 5 takes priority
    // over every item phase and subtracts two directly.
    defense_source_case.cyberware_flags = 0U;
    defense_source_case.inventory_item_codes[0] = 0x27U;
    defense_source_case.inventory_phase_bytes[0] = 8U;
    defense_source_case.inventory_item_codes[1] = 0x27U;
    defense_source_case.inventory_phase_bytes[1] = 2U;
    auto item_phase_defense =
        compute_equipment_combat_bars(defense_source_case);
    if (item_phase_defense.defense_value != 10U) {
        return 33;
    }
    defense_source_case.action_flags = 0x20U;
    item_phase_defense = compute_equipment_combat_bars(defense_source_case);
    if (item_phase_defense.defense_value != 12U) {
        return 34;
    }

    // ROM 0x12E3E adds +0x77 to D6's low byte, not as an unbounded word sum.
    EquipmentCombatInput wrapped_defense_input{};
    wrapped_defense_input.source_mode = 2U;
    wrapped_defense_input.display_state = 0x7F;
    wrapped_defense_input.attributes[0] = 20U;
    wrapped_defense_input.inventory_item_codes[0] = 0x28U;
    wrapped_defense_input.inventory_phase_bytes[0] = 250U;
    wrapped_defense_input.cyberware_flags = 1U << 9U;
    const auto wrapped_defense =
        compute_equipment_combat_bars(wrapped_defense_input);
    if (wrapped_defense.command_average != 250U ||
        wrapped_defense.defense_value != 15U) {
        return 35;
    }

    // A zero command average writes the chosen attack strip and returns
    // before 0x12E12's defense path, including the higher-mode +0x85 case.
    EquipmentCombatInput zero_average_input{};
    zero_average_input.source_mode = 2U;
    zero_average_input.display_state = 0x7F;
    zero_average_input.skills[4] = 7U;
    zero_average_input.posture = 4U;
    const auto zero_average = compute_equipment_combat_bars(zero_average_input);
    if (zero_average.command_average != 0U || !zero_average.attack_written ||
        zero_average.attack_value != 7U || zero_average.attack_bar != 6U ||
        zero_average.defense_written) {
        return 36;
    }

    // The lower-mode action gates differ at 0x17: 0x10 exits before either
    // strip, whereas 0x17 writes the attack and enters defense for nonzero
    // command values. Zero command values again stop after attack.
    auto lower_mode_input = defense_source_case;
    lower_mode_input.source_mode = 0U;
    lower_mode_input.display_state = 0;
    lower_mode_input.action_flags = 0U;
    lower_mode_input.inventory_item_codes = {};
    lower_mode_input.inventory_phase_bytes = {};
    lower_mode_input.source_action_code = 0x10U;
    auto lower_mode_result = compute_equipment_combat_bars(lower_mode_input);
    if (lower_mode_result.attack_written ||
        lower_mode_result.defense_written) {
        return 37;
    }
    lower_mode_input.source_action_code = 0x17U;
    lower_mode_input.skills[4] = 3U;
    lower_mode_result = compute_equipment_combat_bars(lower_mode_input);
    if (!lower_mode_result.attack_written ||
        !lower_mode_result.defense_written) {
        return 38;
    }
    lower_mode_input.attributes[1] = 0U;
    lower_mode_input.attributes[4] = 0U;
    lower_mode_input.attributes[5] = 0U;
    lower_mode_input.source_action_code = 0x17U;
    lower_mode_result = compute_equipment_combat_bars(lower_mode_input);
    if (!lower_mode_result.attack_written ||
        lower_mode_result.defense_written) {
        return 39;
    }

    // The preset sweep reads the three selectable Joshua records and the ten
    // named recruited records from their ROM-extracted raw assets. Expected
    // bar values below are pinned from the ROM's 0x12CEE/0x55ED6/0x55F20
    // branches for every reachable posture, retaining each profile's own
    // class, race, equipped action, skills, inventory, flags, and cyberware.
    const auto asset_catalog = load_conversion_asset_catalog(
        std::filesystem::path(SHADOWRUN_CONVERSION_ASSET_ROOT));
    if (!asset_catalog.complete()) {
        std::cerr << "could not load ROM profile assets for combat-bar audit\n";
        return 40;
    }
    const auto startup_records =
        load_startup_record_assets(asset_catalog.catalog);
    if (!startup_records.complete()) {
        std::cerr << "could not load startup runner records for combat-bar "
                     "audit\n";
        return 41;
    }
    for (std::size_t totem = 0U; totem < kRomTotemActionModifiers.size();
         ++totem) {
        const auto item_code = static_cast<std::uint8_t>(0x29U + totem);
        const auto& item = us_item_catalog()[item_code - 1U];
        for (std::size_t selector = 0U; selector < 3U; ++selector) {
            if (item.source_action_modifier_word(
                    static_cast<std::int8_t>(selector)) !=
                kRomTotemActionModifiers[totem][selector]) {
                std::cerr << "US totem modifier catalog differs from the ROM "
                          << "bytes for item "
                          << static_cast<unsigned>(item_code)
                          << " selector " << selector << '\n';
                return 48;
            }
        }
    }
    std::size_t profile_index = 0U;
    for (const auto& archetype :
         startup_records.assets->joshua_archetypes) {
        LiveRunnerRecord live_record{};
        live_record.bytes = archetype.record;
        if (!check_rom_profile_bars(live_record,
                                    kRomProfileBars[profile_index],
                                    profile_index)) {
            return 42;
        }
        ++profile_index;
    }
    for (std::size_t index = 0U; index < 10U; ++index) {
        LiveRunnerRecord live_record{};
        live_record.bytes =
            startup_records.assets->save_roster_templates[index + 1U].record;
        if (!check_rom_profile_bars(live_record,
                                    kRomProfileBars[profile_index],
                                    profile_index)) {
            return 43;
        }
        ++profile_index;
    }

    // Each of the five ROM spell tables belongs to a different source runner
    // profile. Sweep every listed spell, each legal force, and all five
    // posture values through the command overview's special-action bars.
    // Table entries are the source's force limits from 0x166AD8; the static
    // profile bar rows above anchor the paired defense result.
    std::array<LiveRunnerRecord, 5U> caster_records{};
    caster_records[0U].bytes =
        startup_records.assets->save_roster_templates[1U].record;
    caster_records[1U].bytes =
        startup_records.assets->save_roster_templates[3U].record;
    caster_records[2U].bytes =
        startup_records.assets->save_roster_templates[5U].record;
    caster_records[3U].bytes =
        startup_records.assets->save_roster_templates[8U].record;
    caster_records[4U].bytes =
        startup_records.assets->joshua_archetypes[2U].record;
    constexpr std::array<std::size_t, 5U> kCasterProfileIndices = {
        3U, 5U, 7U, 10U, 2U};
    std::size_t spell_force_posture_cases = 0U;
    for (std::size_t table_index = 0U;
         table_index < caster_records.size(); ++table_index) {
        const auto fields = read_runner_record_fields(caster_records[table_index]);
        const auto* spell_table = find_us_magic_spell_table(
            static_cast<std::uint16_t>(table_index));
        if (spell_table == nullptr ||
            fields.magic_table_index != table_index ||
            fields.archetype_index > 1U || fields.auxiliary[0U] == 0U ||
            spell_table->entry_count == 0U) {
            std::cerr << "ROM caster profile/table mismatch at table "
                      << table_index << '\n';
            return 44;
        }

        for (std::size_t entry_index = 0U;
             entry_index < spell_table->entry_count; ++entry_index) {
            const auto& entry = spell_table->entries[entry_index];
            const auto* spell = find_us_magic_spell(entry.spell_code);
            if (spell == nullptr || entry.force_limit == 0U) {
                std::cerr << "ROM spell table has an invalid entry at table "
                          << table_index << " entry " << entry_index << '\n';
                return 45;
            }
            for (std::uint8_t force = 1U; force <= entry.force_limit; ++force) {
                for (std::uint8_t posture = 0U; posture <= 4U; ++posture) {
                    const auto combat_input = make_spell_bar_input(
                        caster_records[table_index], *spell, force, posture);
                    const auto bars =
                        compute_equipment_combat_bars(combat_input);
                    const auto expected_attack = expected_rom_spell_attack_value(
                        caster_records[table_index], entry.spell_code, force,
                        posture);
                    const auto profile_row =
                        kCasterProfileIndices[table_index];
                    const auto expected_defense =
                        kRomProfileBars[profile_row].defense_values[posture];
                    if (!bars.attack_written || !bars.defense_written ||
                        bars.source_data_missing ||
                        bars.command_average !=
                            kRomProfileBars[profile_row].command_average ||
                        bars.attack_value != expected_attack ||
                        bars.attack_bar != expected_attack - 1U ||
                        bars.defense_value != expected_defense ||
                        bars.defense_bar != expected_defense - 1U ||
                        !bars.special_action.menu_service_called ||
                        bars.special_action.magic_spell_missing ||
                        !bars.special_action.residual_calculated) {
                        std::cerr << "ROM spell combat-bar permutation mismatch"
                                  << " table=" << table_index
                                  << " spell="
                                  << static_cast<unsigned>(entry.spell_code)
                                  << " force=" << static_cast<unsigned>(force)
                                  << " posture="
                                  << static_cast<unsigned>(posture)
                                  << " attack=" << bars.attack_value << "/"
                                  << static_cast<unsigned>(bars.attack_bar)
                                  << " defense=" << bars.defense_value << "/"
                                  << static_cast<unsigned>(bars.defense_bar)
                                  << '\n';
                        return 46;
                    }
                    ++spell_force_posture_cases;
                }
            }
        }
    }

    // Section 06 frames 000015-000020 select Gator Shaman Mana Zap at force
    // three and Neutral posture. The parent command page's section-06 frame
    // 002550 shows Atk row 5 and Def row 4, matching this source-derived pair.
    const auto gator_mana_zap = compute_equipment_combat_bars(
        make_spell_bar_input(caster_records[4U], *find_us_magic_spell(7U),
                             3U, 2U));
    if (spell_force_posture_cases != 260U ||
        gator_mana_zap.command_average != 5U ||
        gator_mana_zap.attack_value != 6U ||
        gator_mana_zap.attack_bar != 5U ||
        gator_mana_zap.defense_value != 5U ||
        gator_mana_zap.defense_bar != 4U ||
        gator_mana_zap.source_data_missing) {
        return 47;
    }

    MagicActionEffectValueInput metric_input{};
    metric_input.current_force = 2U;
    metric_input.entity.item18_base = 10U;
    metric_input.spell = *find_us_magic_spell(2U);
    std::vector<int> metric_events;
    const auto metric = refresh_magic_action_effect_value(
        metric_input,
        {[&](const MagicActionEffectNoneValuePlan&) {
             metric_events.push_back(1);
         },
         [&](const MagicActionEffectDamageGridPlan& plan) {
             metric_events.push_back(static_cast<int>(plan.effect_row));
         }});
    if (!metric.residual_valid || metric.residual_effect != 7 ||
        !metric.damage_grid_written || metric.none_damage_value_rendered ||
        !metric.damage_grid.has_value() ||
        metric.damage_grid->effect_row != 6U ||
        metric.damage_grid->source_row_stride != 0x0EU ||
        metric.damage_grid->destination_tile_base != 0xC688U ||
        metric.damage_grid->tile_count != 7U ||
        metric_events != std::vector<int>{6}) {
        return 8;
    }

    metric_input.spell = *find_us_magic_spell(0U);
    metric_events.clear();
    const auto no_effect = refresh_magic_action_effect_value(
        metric_input,
        {[&](const MagicActionEffectNoneValuePlan& plan) {
             metric_events.push_back(
                 plan.destination_tile_base == 0xC68CU ? 1 : 2);
         },
         [&](const MagicActionEffectDamageGridPlan& plan) {
             metric_events.push_back(static_cast<int>(plan.effect_row));
         }});
    if (!no_effect.residual_valid || no_effect.residual_effect != -4 ||
        !no_effect.none_damage_value_rendered ||
        no_effect.damage_grid_written || !no_effect.none_damage_value.has_value() ||
        no_effect.none_damage_value->destination_tile_base != 0xC68CU ||
        metric_events != std::vector<int>{1}) {
        return 9;
    }

    MagicMenuValueRefreshInput combat_value_input{};
    combat_value_input.current_force = 2U;
    combat_value_input.action_cap = 5U;
    combat_value_input.active_entity.item18_base = 12U;
    combat_value_input.active_entity.posture = 2U;
    std::vector<int> combat_value_events;
    MagicMenuValueRefreshCallbacks combat_value_callbacks{};
    combat_value_callbacks.resolve_menu_metric =
        [&](const std::uint16_t value) {
            combat_value_events.push_back(1);
            return std::optional<std::uint16_t>{
                static_cast<std::uint16_t>(value + 3U)};
        };
    combat_value_callbacks.write_grid =
        [&](const MagicMenuValueGridPlan& plan) {
            const auto offset = plan.strip == MagicMenuValueStrip::success
                                     ? 10U
                                     : plan.strip ==
                                               MagicMenuValueStrip::defense_physical
                                           ? 20U
                                           : 30U;
            combat_value_events.push_back(
                static_cast<int>(plan.row + offset));
        };
    const auto combat_value_result =
        refresh_magic_menu_success_or_defense_values(
        combat_value_input, combat_value_callbacks);
    if (!(combat_value_result.menu_metric_available &&
          combat_value_result.success_value == 10U &&
          combat_value_result.defense_physical_value == 0U &&
          combat_value_result.defense_mana_value == 0U &&
          combat_value_result.grid_requests.size() == 1U &&
          combat_value_result.grid_requests[0].strip ==
              MagicMenuValueStrip::success &&
          combat_value_result.grid_requests[0].row == 9U &&
          combat_value_result.grid_requests[0].destination_tile_base ==
              0xC588U &&
          combat_value_result.grid_requests[0].source_row_stride == 0x1AU &&
          combat_value_result.grid_requests[0].tile_count == 13U &&
          combat_value_events == std::vector<int>{1, 19})) {
        return 10;
    }

    combat_value_input = {};
    combat_value_input.refresh_defense_strips = true;
    combat_value_input.attribute_77 = 4U;
    combat_value_input.attribute_7c = 2U;
    combat_value_input.ready_entities[0].flags_10 = 1U;
    combat_value_input.ready_entities[0].class_code = 0U;
    combat_value_input.ready_entities[0].item18_base = 8U;
    combat_value_input.ready_entities[1].flags_10 = 1U;
    combat_value_input.ready_entities[1].class_code = 1U;
    combat_value_input.ready_entities[1].item18_base = 5U;
    combat_value_events.clear();
    int ready_refreshes = 0;
    combat_value_callbacks.resolve_menu_metric = {};
    combat_value_callbacks.refresh_ready_entity_animations =
        [&] { ++ready_refreshes; };
    const auto alternate_values =
        refresh_magic_menu_success_or_defense_values(
        combat_value_input, combat_value_callbacks);
    if (!(ready_refreshes == 1 &&
          alternate_values.success_value == 0U &&
          alternate_values.defense_physical_value == 7U &&
          alternate_values.defense_mana_value == 5U &&
          alternate_values.grid_requests.size() == 2U &&
          alternate_values.grid_requests[0].strip ==
              MagicMenuValueStrip::defense_physical &&
          alternate_values.grid_requests[0].row == 6U &&
          alternate_values.grid_requests[0].destination_tile_base ==
              0xC808U &&
          alternate_values.grid_requests[1].strip ==
              MagicMenuValueStrip::defense_mana &&
          alternate_values.grid_requests[1].row == 4U &&
          alternate_values.grid_requests[1].destination_tile_base ==
              0xC908U &&
          combat_value_events == std::vector<int>{26, 34})) {
        return 11;
    }

    EquipmentHighlightInput highlight_input{};
    highlight_input.equipped_display_state = 2;
    highlight_input.highlight_item = 1U;
    highlight_input.inventory_item_codes[1] = 0x2CU;
    highlight_input.inventory_item_codes[6] = 0x2CU;
    std::vector<int> highlight_events;
    const EquipmentHighlightCallbacks highlight_callbacks{
        [&] { highlight_events.push_back(1); },
        [&] { highlight_events.push_back(2); },
        [&] { highlight_events.push_back(3); },
    };
    const auto highlights = render_equipment_highlights(
        highlight_input, highlight_callbacks);
    if (!(highlights.equipped_grid_written &&
          highlights.inventory_grid_written &&
          highlights.matched_inventory_slot == 6 &&
          highlight_events == std::vector<int>{1, 2, 2, 3})) {
        return 12;
    }
    highlight_input.equipped_display_state = -1;
    highlight_input.highlight_item = 0U;
    highlight_events.clear();
    const auto no_highlights = render_equipment_highlights(
        highlight_input, highlight_callbacks);
    if (!(!no_highlights.equipped_grid_written &&
          !no_highlights.inventory_grid_written &&
          highlight_events == std::vector<int>{1, 3})) {
        return 13;
    }

    // ROM 0x12A8C: ordinary item action details render art, the item name,
    // phase overlays in source order, the action label, and the detail value.
    ResourceVariantState item_variant{};
    ItemActionDetailInput item_detail_input{};
    item_detail_input.item_code = 12U;
    item_detail_input.item_art_available = true;
    item_detail_input.item_detail_value = 5U;
    item_detail_input.phase_bytes[8] = 0x13U; // bits 1, 0, and 4
    std::vector<int> item_detail_events;
    int item_action_reads = 0;
    ItemActionDetailCallbacks item_detail_callbacks{};
    item_detail_callbacks.write_grid =
        [&](const ItemActionDetailGridPlan& plan) {
            item_detail_events.push_back(
                static_cast<int>(plan.kind) + 1);
        };
    item_detail_callbacks.render_text =
        [&](const ItemActionDetailTextPlan& plan) {
            item_detail_events.push_back(
                plan.kind == ItemActionDetailTextKind::item_name ? 10 : 11);
        };
    item_detail_callbacks.read_entity_action_code = [&] {
        ++item_action_reads;
        return std::uint8_t{5U};
    };
    const auto ordinary_item = render_item_action_details(
        item_detail_input, item_variant, item_detail_callbacks);
    if (!(ordinary_item.returned_item_code == 12U &&
          ordinary_item.action_code == std::optional<std::uint8_t>{5U} &&
          ordinary_item.label == ActionDetailLabelVariant::ordinary &&
          ordinary_item.phase_slot_used == 8U && item_action_reads == 1 &&
          ordinary_item.item_art_written && ordinary_item.item_name_rendered &&
          ordinary_item.action_label_rendered &&
          ordinary_item.detail_value_written &&
          ordinary_item.grid_requests.size() == 5U &&
          ordinary_item.text_requests.size() == 2U &&
          ordinary_item.grid_requests[1].phase_bit == 1U &&
          ordinary_item.grid_requests[1].destination_tile_base == 0xC19CU &&
          ordinary_item.grid_requests[2].phase_bit == 0U &&
          ordinary_item.grid_requests[2].destination_tile_base == 0xC1B6U &&
          ordinary_item.grid_requests[3].phase_bit == 4U &&
          ordinary_item.grid_requests[3].destination_tile_base == 0xC336U &&
          ordinary_item.grid_requests[4].detail_value == 5U &&
          ordinary_item.text_requests[0].item_record_index == 11U &&
          ordinary_item.text_requests[1].label ==
              ActionDetailLabelVariant::ordinary &&
          item_detail_events == std::vector<int>{1, 10, 2, 2, 2, 11, 3})) {
        return 14;
    }

    // The source-bound path ignores caller guesses: item 12 supplies its
    // D4540 +0x1E art presence and +0x24 value (0x08), not the stale value 5
    // above.
    item_variant = {};
    item_detail_events.clear();
    item_action_reads = 0;
    item_detail_input.item_art_available = false;
    item_detail_input.item_detail_value = 0xEEU;
    const auto typed_ordinary_item = render_item_action_details(
        item_detail_input, us_item_catalog(), item_variant,
        item_detail_callbacks);
    if (!(typed_ordinary_item.item_art_written &&
          typed_ordinary_item.grid_requests.size() == 5U &&
          typed_ordinary_item.grid_requests.back().detail_value ==
              us_item_catalog()[11].source_combat_factor &&
          typed_ordinary_item.grid_requests.back().detail_value == 0x08U &&
          typed_ordinary_item.text_requests.size() == 2U &&
          item_action_reads == 1 &&
          item_detail_events == std::vector<int>{1, 10, 2, 2, 2, 11, 3})) {
        return 30;
    }

    // Rows 41..43 have a source +0x1E pointer of -1. The typed path must not
    // render detail art even if the generic adapter input claims it exists.
    item_variant = {};
    item_detail_events.clear();
    item_action_reads = 0;
    ItemActionDetailInput no_art_input{};
    no_art_input.item_code = 42U;
    no_art_input.item_art_available = true;
    no_art_input.item_detail_value = 0xEEU;
    const auto typed_no_art_item = render_item_action_details(
        no_art_input, us_item_catalog(), item_variant, item_detail_callbacks);
    if (!(typed_no_art_item.item_name_rendered &&
          !typed_no_art_item.item_art_written &&
          typed_no_art_item.grid_requests.size() == 1U &&
          typed_no_art_item.grid_requests.back().detail_value == 0U &&
          item_action_reads == 1)) {
        return 31;
    }

    // A code outside the typed catalog is an explicit missing source row; it
    // must not enter the resource-variant or presentation path.
    item_variant = {};
    item_detail_events.clear();
    item_action_reads = 0;
    ItemActionDetailInput missing_item_input{};
    missing_item_input.item_code = 51U;
    const auto missing_item = render_item_action_details(
        missing_item_input, us_item_catalog(), item_variant,
        item_detail_callbacks);
    if (!(missing_item.item_record_missing &&
          missing_item.returned_item_code == 51U && item_action_reads == 0 &&
          item_detail_events.empty() && !item_variant.alternate)) {
        return 32;
    }

    // Codes 0x0D..0x13 use the extended label table and perform the second
    // 0x12148 lookup before selecting the phase byte as the value.
    item_variant = {};
    item_detail_input = {};
    item_detail_input.item_code = 13U;
    item_detail_input.phase_slot = 2U;
    item_detail_input.phase_bytes[2] = 0xA5U;
    item_detail_events.clear();
    item_action_reads = 0;
    int item_uploads = 0;
    item_detail_callbacks.upload_resource_stream = [&] {
        ++item_uploads;
    };
    item_detail_callbacks.read_entity_action_code = [&] {
        ++item_action_reads;
        return item_action_reads == 1 ? std::uint8_t{0x0DU}
                                      : std::uint8_t{0x21U};
    };
    const auto extended_item = render_item_action_details(
        item_detail_input, item_variant, item_detail_callbacks);
    if (!(extended_item.resource_variant_changed && item_variant.alternate &&
          item_uploads == 1 && extended_item.action_code ==
                                    std::optional<std::uint8_t>{0x0DU} &&
          extended_item.label_action_code ==
              std::optional<std::uint8_t>{0x21U} &&
          extended_item.label == ActionDetailLabelVariant::extended &&
          extended_item.phase_slot_used == 2U && item_action_reads == 2 &&
          extended_item.grid_requests.size() == 1U &&
          extended_item.grid_requests[0].kind ==
              ItemActionDetailGridKind::detail_value &&
          extended_item.grid_requests[0].detail_value == 0xA5U &&
          extended_item.text_requests.size() == 2U &&
          extended_item.text_requests[1].action_code ==
              std::optional<std::uint8_t>{0x21U})) {
        return 15;
    }

    // 0x15..0x17 return after the first action read, without a label/value
    // pair; the art and name work has already happened.
    item_variant = {};
    item_detail_input = {};
    item_detail_input.item_code = 10U;
    item_detail_input.item_art_available = true;
    item_detail_events.clear();
    item_action_reads = 0;
    item_detail_callbacks.read_entity_action_code = [&] {
        ++item_action_reads;
        return std::uint8_t{0x16U};
    };
    const auto early_item = render_item_action_details(
        item_detail_input, item_variant, item_detail_callbacks);
    if (!(early_item.early_return && !early_item.action_label_rendered &&
          !early_item.detail_value_written && item_action_reads == 1 &&
          early_item.grid_requests.size() == 1U &&
          early_item.text_requests.size() == 1U)) {
        return 16;
    }

    // 0x2C and above transfer to armor selection rather than drawing the
    // action label/value pair.
    item_variant = {};
    item_detail_input = {};
    item_detail_input.item_code = 20U;
    item_action_reads = 0;
    bool armor_preview = false;
    item_detail_callbacks.read_entity_action_code = [&] {
        ++item_action_reads;
        return std::uint8_t{0x2CU};
    };
    item_detail_callbacks.apply_armor_selection =
        [&](const bool preview) { armor_preview = preview; };
    const auto armor_item = render_item_action_details(
        item_detail_input, item_variant, item_detail_callbacks);
    if (!(armor_item.armor_selection_requested && armor_preview &&
          item_action_reads == 1 && !armor_item.action_label_rendered &&
          !armor_item.detail_value_written &&
          armor_item.text_requests.size() == 1U)) {
        return 17;
    }

    // ROM 0x12EAE: with no active action and no cyberware bit, the source
    // renders its header, selects asset one, then emits the detail row.
    EquipmentActionDetailRowCallbacks row_callbacks{};
    std::vector<int> row_events;
    row_callbacks.render_action_header =
        [&](const EquipmentActionDetailHeaderPlan& plan) {
            row_events.push_back(plan.destination_tile_base == 0xC538U ? 1
                                                                        : 99);
        };
    row_callbacks.advance_entity_action_menu = [&] { row_events.push_back(5); };
    row_callbacks.publish_action_selection =
        [&](const ActionDetailRenderPlan& plan) {
            row_events.push_back(plan.valid ? 2 : 99);
        };
    row_callbacks.render_detail_text =
        [&](const EquipmentActionDetailTextPlan& plan) {
            row_events.push_back(plan.destination_tile_base == 0xC51CU ? 3
                                                                        : 99);
        };
    row_callbacks.write_grid = [&](const EquipmentActionDetailGridPlan& plan) {
        row_events.push_back(plan.destination_tile_base == 0xC52AU ? 4 : 99);
    };
    EquipmentActionDetailRowInput row_input{};
    row_input.selected_action_code = 0U;
    row_input.cyberware_base = 2U;
    const auto no_active_row = render_equipment_action_detail_row(
        row_input, row_callbacks);
    if (!(no_active_row.action_header_rendered &&
          no_active_row.action_selection.has_value() &&
          no_active_row.action_selection->source_code == 2U &&
          no_active_row.detail_grid.has_value() &&
          no_active_row.detail_grid->source_table ==
              EquipmentActionDetailAssetTable::no_active_action &&
          no_active_row.detail_grid->asset_index == 1U &&
          no_active_row.detail_grid->inner_loop_counter == 6U &&
          row_events == std::vector<int>{1, 2, 3, 4})) {
        return 18;
    }

    // An ordinary active action reads the normal +0x25 asset row and the
    // separate +0x26 label selector before the final detail text/grid.
    row_events.clear();
    row_input = {};
    row_input.source_mode = 0U;
    row_input.active_action_code = 2U;
    row_input.selected_action_code = 5U;
    row_input.action_detail_code = 4U;
    row_input.detail_asset_index = 3;
    const auto normal_row = render_equipment_action_detail_row(
        row_input, row_callbacks);
    if (!(normal_row.action_selection.has_value() &&
          normal_row.action_selection->source_code == 4U &&
          normal_row.detail_grid.has_value() &&
          normal_row.detail_grid->source_table ==
              EquipmentActionDetailAssetTable::normal_item_records &&
          normal_row.detail_grid->asset_index == 3U &&
          !normal_row.action_header_rendered && !normal_row.action_menu_advanced &&
          row_events == std::vector<int>{2, 3, 4})) {
        return 19;
    }

    row_events.clear();
    const auto typed_normal_row = render_equipment_action_detail_row(
        row_input, us_item_catalog(), row_callbacks);
    if (!(typed_normal_row.action_selection.has_value() &&
          typed_normal_row.detail_grid.has_value() &&
          typed_normal_row.detail_grid->asset_index == 0U &&
          !typed_normal_row.detail_asset_missing &&
          row_events == std::vector<int>{2, 3, 4})) {
        return 28;
    }

    row_input.active_action_code = 51U;
    row_events.clear();
    const auto missing_typed_item = render_equipment_action_detail_row(
        row_input, us_item_catalog(), row_callbacks);
    if (!(missing_typed_item.detail_asset_missing &&
          !missing_typed_item.action_selection.has_value() &&
          row_events.empty())) {
        return 29;
    }

    // Higher-mode negative display state uses the alternate action table,
    // renders the header for active action five, and advances the action menu.
    row_events.clear();
    row_input = {};
    row_input.source_mode = 2U;
    row_input.display_state = -1;
    row_input.active_action_code = 5U;
    row_input.auxiliary_value = 9U;
    row_input.magic_spell = *find_us_magic_spell(4U);
    row_input.detail_asset_index = 2;
    const auto alternate_row = render_equipment_action_detail_row(
        row_input, row_callbacks);
    if (!(alternate_row.action_header_rendered &&
          alternate_row.action_menu_advanced &&
          alternate_row.action_selection.has_value() &&
          alternate_row.action_selection->source_code == 9U &&
          alternate_row.detail_grid.has_value() &&
          alternate_row.detail_grid->source_table ==
              EquipmentActionDetailAssetTable::alternate_action_records &&
          alternate_row.detail_grid->asset_index == 1U &&
          row_events == std::vector<int>{1, 5, 2, 3, 4})) {
        return 20;
    }

    row_input.active_action_code = 4U;
    row_input.magic_spell = *find_us_magic_spell(3U);
    row_input.detail_asset_index = 1;
    row_events.clear();
    const auto skipped_row = render_equipment_action_detail_row(
        row_input, row_callbacks);
    if (!(skipped_row.early_return && !skipped_row.action_selection.has_value() &&
          !skipped_row.action_header_rendered &&
          row_events == std::vector<int>{5})) {
        return 21;
    }

    row_input.active_action_code = 5U;
    row_input.magic_spell = *find_us_magic_spell(3U);
    row_input.detail_asset_index = 1;
    row_events.clear();
    const auto mismatched_magic_row = render_equipment_action_detail_row(
        row_input, row_callbacks);
    if (!(mismatched_magic_row.detail_asset_missing &&
          !mismatched_magic_row.action_selection.has_value() &&
          row_events == std::vector<int>{1, 5})) {
        return 27;
    }

    row_input = {};
    row_input.active_action_code = 2U;
    row_input.selected_action_code = 5U;
    row_input.detail_asset_index = 3;
    const auto missing_label_row = render_equipment_action_detail_row(
        row_input, row_callbacks);
    if (!(missing_label_row.action_selection_missing &&
          !missing_label_row.detail_text_rendered &&
          !missing_label_row.detail_grid_written)) {
        return 22;
    }

    // ROM 0x111E8: an unset refresh marker delegates to the existing menu-row
    // renderer without touching the equipment-details sequence.
    EquipmentDetailsCallbacks details_callbacks{};
    std::vector<int> details_events;
    details_callbacks.render_existing_menu_row =
        [&] { details_events.push_back(1); };
    details_callbacks.prepare_menu_grid_variant_b =
        [&] { details_events.push_back(2); };
    details_callbacks.render_text = [&](const EquipmentDetailsTextPlan& plan) {
        details_events.push_back(10 + static_cast<int>(plan.kind));
    };
    details_callbacks.build_meter_glyphs =
        [&](const EquipmentMeterPlan& plan) {
            details_events.push_back(
                plan.kind == EquipmentMeterKind::physical ? 20 : 21);
        };
    details_callbacks.apply_armor_selection =
        [&] { details_events.push_back(30); };
    details_callbacks.prepare_menu_dialog = [&] { details_events.push_back(31); };
    details_callbacks.action_row_callbacks.publish_action_selection =
        [&](const ActionDetailRenderPlan&) { details_events.push_back(40); };
    details_callbacks.action_row_callbacks.render_detail_text =
        [&](const EquipmentActionDetailTextPlan&) { details_events.push_back(41); };
    details_callbacks.action_row_callbacks.write_grid =
        [&](const EquipmentActionDetailGridPlan&) { details_events.push_back(42); };

    const auto stale_details = render_equipment_details(
        EquipmentDetailsInput{}, details_callbacks);
    if (!(stale_details.existing_menu_row_rendered &&
          !stale_details.menu_grid_prepared && details_events ==
              std::vector<int>{1})) {
        return 23;
    }

    // A refreshed no-action page lays out heading/race/class/cyberware text,
    // invokes 0x12EAE, then emits both meters and the closing services.
    details_events.clear();
    EquipmentDetailsInput details_input{};
    details_input.refresh_requested = true;
    details_input.race_or_state = 3U;
    details_input.class_code = 4U;
    details_input.physical_meter = 2U;
    details_input.mental_meter = 6U;
    details_input.action_row.source_mode = 0U;
    details_input.action_row.selected_action_code = 0U;
    details_input.action_row.cyberware_base = 2U;
    details_input.action_row.cyberware_flags = static_cast<std::uint16_t>(
        1U << 3U);
    const auto no_action_details = render_equipment_details(
        details_input, details_callbacks);
    if (!(no_action_details.refresh_flag_cleared &&
          no_action_details.menu_grid_prepared &&
          no_action_details.action_detail_row_invoked &&
          no_action_details.action_detail_row.has_value() &&
          no_action_details.action_detail_row->action_selection.has_value() &&
          no_action_details.action_detail_row->action_selection->source_code ==
              2U && no_action_details.text_requests.size() == 4U &&
          no_action_details.text_requests[3].kind ==
              EquipmentDetailsTextKind::cyberware_action_bit3 &&
          no_action_details.meter_requests.size() == 2U &&
          no_action_details.physical_meter_rendered &&
          no_action_details.mental_meter_rendered &&
          no_action_details.armor_selection_applied &&
          no_action_details.dialog_prepared &&
          details_events == std::vector<int>{2, 10, 11, 12, 13, 40, 41, 42,
                                             20, 21, 30, 31})) {
        return 24;
    }

    // Action code 0x10 fails the wrapped CMP.B #0x10 detail-row gate while
    // still completing the meters and dialog sequence.
    details_events.clear();
    details_input.action_row.active_action_code = 0x10U;
    details_input.action_row.display_state = 0;
    const auto high_action_details = render_equipment_details(
        details_input, details_callbacks);
    if (!(high_action_details.text_requests.size() == 4U &&
          high_action_details.text_requests[3].kind ==
              EquipmentDetailsTextKind::active_action_normal &&
          !high_action_details.action_detail_row_invoked &&
          high_action_details.meter_requests.size() == 2U &&
          details_events == std::vector<int>{2, 10, 11, 12, 16, 20, 21, 30,
                                             31})) {
        return 25;
    }

    // A low active action selects its +0x26 label row and +0x25 detail asset
    // through the nested 0x12EAE conversion.
    details_events.clear();
    details_input.action_row.active_action_code = 2U;
    details_input.action_row.selected_action_code = 5U;
    details_input.action_row.action_detail_code = 4U;
    details_input.action_row.detail_asset_index = 3;
    const auto active_action_details = render_equipment_details(
        details_input, details_callbacks);
    if (!(active_action_details.action_detail_row_invoked &&
          active_action_details.action_detail_row.has_value() &&
          active_action_details.action_detail_row->action_selection.has_value() &&
          active_action_details.action_detail_row->action_selection->source_code ==
              4U && active_action_details.text_requests[3].kind ==
              EquipmentDetailsTextKind::active_action_normal &&
          details_events == std::vector<int>{2, 10, 11, 12, 16, 40, 41, 42,
                                             20, 21, 30, 31})) {
        return 26;
    }

    // The page-level source-bound overload passes the same active-action row
    // into 0x12EAE, so item 2 supplies D4540 +0x25 == 0 instead of the stale
    // adapter value 3 above.
    details_events.clear();
    const auto typed_active_action_details = render_equipment_details(
        details_input, us_item_catalog(), details_callbacks);
    if (!(typed_active_action_details.action_detail_row_invoked &&
          typed_active_action_details.action_detail_row.has_value() &&
          typed_active_action_details.action_detail_row->detail_grid.has_value() &&
          typed_active_action_details.action_detail_row->detail_grid->asset_index ==
              0U &&
          !typed_active_action_details.action_detail_row->detail_asset_missing &&
          details_events == std::vector<int>{2, 10, 11, 12, 16, 40, 41, 42,
                                             20, 21, 30, 31})) {
        return 33;
    }

    return 0;
}
