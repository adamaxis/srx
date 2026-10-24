#include "shadowrun_ghidra/menus/magic_menu_conversion.hpp"

#include <array>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion;

constexpr std::size_t kRecordSize = 0x100U;
constexpr std::size_t kJoshuaOffset = 0x7291AU;
constexpr std::size_t kRecruitedOffset = 0x72C1AU;
constexpr std::size_t kRecruitedCount = 10U;
constexpr std::size_t kPartySlots = 3U;
constexpr std::size_t kInventorySlots = 8U;
constexpr std::uint8_t kPowerFocus = 0x18U;

constexpr std::array<std::array<std::uint8_t, 2U>, 13U>
    kSourceProfileIdentity{{
        {{0U, 2U}}, {{0U, 3U}}, {{0U, 1U}},
        {{2U, 1U}}, {{4U, 2U}}, {{0U, 0U}}, {{2U, 3U}}, {{3U, 1U}},
        {{1U, 3U}}, {{0U, 2U}}, {{1U, 0U}}, {{1U, 3U}}, {{0U, 2U}},
    }};

bool fail(const std::string& message) {
    std::cerr << "Magic Defense party permutation: " << message << '\n';
    return false;
}

bool read_file(const std::filesystem::path& path,
               std::vector<std::uint8_t>& bytes) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        return false;
    }
    bytes.assign(std::istreambuf_iterator<char>(input),
                 std::istreambuf_iterator<char>());
    return !input.bad();
}

bool decode_source_record(const std::vector<std::uint8_t>& rom,
                          const std::size_t offset,
                          EntityActionRecord& record,
                          std::uint8_t& race,
                          std::uint8_t& runner_class) {
    if (offset > rom.size() || kRecordSize > rom.size() - offset) {
        return false;
    }
    race = rom[offset + 0x9AU];
    runner_class = rom[offset + 0x9BU];
    record.race_or_state = static_cast<std::int8_t>(race);
    record.class_code = runner_class;
    record.flags_10 = rom[offset + 0x10U];
    record.attribute_77 = rom[offset + 0x77U];
    record.attribute_7c = rom[offset + 0x7CU];
    record.item18_base = rom[offset + 0x80U];
    for (std::size_t slot = 0U; slot < kInventorySlots; ++slot) {
        record.item_codes[slot] = rom[offset + 0x66U + slot];
        record.item_phases[slot] = rom[offset + 0x6EU + slot];
    }
    return true;
}

std::uint16_t expected_source_party_pool(
    const std::array<EntityActionRecord, kPartySlots>& party) {
    // Independent test oracle for ROM 0x1047E/0x55F02. The source adds the
    // first item-0x18 phase found from slot 6 down through slot 0 using a
    // low-byte add, then rounds each eligible caster to a quarter.
    std::uint16_t pool = 0U;
    for (const auto& member : party) {
        if ((member.flags_10 & 0x01U) == 0U || member.class_code >= 2U) {
            continue;
        }
        auto skill = member.item18_base;
        for (std::size_t slot = 7U; slot-- > 0U;) {
            if (member.item_codes[slot] == kPowerFocus) {
                skill = static_cast<std::uint8_t>(
                    static_cast<std::uint8_t>(skill) +
                    member.item_phases[slot]);
                break;
            }
        }
        pool = static_cast<std::uint16_t>(pool + skill / 4U);
        if ((skill % 4U) >= 3U) {
            ++pool;
        }
    }
    return pool;
}

std::uint8_t expected_strip_value(const std::uint16_t pool,
                                  const std::uint8_t attribute) {
    return static_cast<std::uint8_t>(pool + attribute);
}

std::uint16_t expected_strip_row(const std::uint8_t value) {
    return value == 0U ? 0U : static_cast<std::uint16_t>(value - 1U);
}

bool check_strip_pair(
    const std::array<EntityActionRecord, kPartySlots>& party,
    const EntityActionRecord& active_runner,
    const std::string& label) {
    const auto pool = expected_source_party_pool(party);
    const auto expected_physical =
        expected_strip_value(pool, active_runner.attribute_77);
    const auto expected_mana =
        expected_strip_value(pool, active_runner.attribute_7c);
    const auto physical = prepare_magic_fallback_defense_strip(
        party, active_runner, false);
    const auto mana = prepare_magic_fallback_defense_strip(
        party, active_runner, true);
    MagicMenuValueRefreshInput spell_panel_input{};
    spell_panel_input.refresh_defense_strips = true;
    spell_panel_input.active_entity = active_runner;
    spell_panel_input.ready_entities = party;
    spell_panel_input.attribute_77 = active_runner.attribute_77;
    spell_panel_input.attribute_7c = active_runner.attribute_7c;
    const auto spell_panel = refresh_magic_menu_success_or_defense_values(
        spell_panel_input, {});
    const auto expected_spell_physical = expected_physical;
    const auto expected_spell_mana = expected_mana;
    const auto expected_spell_grid_count =
        expected_spell_mana == 0U ? 1U : 2U;
    if (physical.strip != MagicMenuValueStrip::defense_physical ||
        physical.value != expected_physical ||
        physical.row != expected_strip_row(expected_physical) ||
        physical.destination_tile_base != kMagicFallbackPhysicalDestination ||
        physical.palette_flags != kMagicFallbackDefensePalette ||
        mana.strip != MagicMenuValueStrip::defense_mana ||
        mana.value != expected_mana ||
        mana.row != expected_strip_row(expected_mana) ||
        mana.destination_tile_base != kMagicFallbackManaDestination ||
        mana.palette_flags != kMagicFallbackDefensePalette ||
        spell_panel.defense_physical_value != expected_spell_physical ||
        spell_panel.defense_mana_value != expected_spell_mana ||
        spell_panel.grid_requests.size() != expected_spell_grid_count ||
        spell_panel.grid_requests[0].strip !=
            MagicMenuValueStrip::defense_physical ||
        spell_panel.grid_requests[0].value != expected_spell_physical ||
        spell_panel.grid_requests[0].row !=
            expected_strip_row(expected_spell_physical) ||
        spell_panel.grid_requests[0].destination_tile_base != 0xC808U ||
        (expected_spell_mana != 0U &&
         (spell_panel.grid_requests[1].strip !=
              MagicMenuValueStrip::defense_mana ||
          spell_panel.grid_requests[1].value != expected_spell_mana ||
          spell_panel.grid_requests[1].row !=
              expected_strip_row(expected_spell_mana) ||
          spell_panel.grid_requests[1].destination_tile_base != 0xC908U))) {
        return fail(label + " produced Physical " +
                    std::to_string(physical.value) + "/row " +
                    std::to_string(physical.row) + " and Mana " +
                    std::to_string(mana.value) + "/row " +
                    std::to_string(mana.row) + "; expected " +
                    std::to_string(expected_physical) + "/row " +
                    std::to_string(expected_strip_row(expected_physical)) +
                    " and " + std::to_string(expected_mana) + "/row " +
                    std::to_string(expected_strip_row(expected_mana)) +
                    "; spell panel physical/mana " +
                    std::to_string(spell_panel.defense_physical_value) + "/" +
                    std::to_string(spell_panel.defense_mana_value) +
                    " issued " +
                    std::to_string(spell_panel.grid_requests.size()) +
                    " grids");
    }
    return true;
}

} // namespace

int main() {
#ifndef SHADOWRUN_US_ROM_PATH
    return 1;
#else
    std::vector<std::uint8_t> rom;
    if (!read_file(SHADOWRUN_US_ROM_PATH, rom)) {
        fail("could not read the US Genesis ROM");
        return 2;
    }

    std::array<EntityActionRecord, 13U> source_profiles{};
    for (std::size_t profile = 0U; profile < source_profiles.size(); ++profile) {
        const auto offset = profile < 3U
            ? kJoshuaOffset + profile * kRecordSize
            : kRecruitedOffset + (profile - 3U) * kRecordSize;
        std::uint8_t race = 0U;
        std::uint8_t runner_class = 0U;
        if (!decode_source_record(rom, offset, source_profiles[profile],
                                  race, runner_class) ||
            race != kSourceProfileIdentity[profile][0] ||
            runner_class != kSourceProfileIdentity[profile][1]) {
            fail("source profile identity changed at template " +
                 std::to_string(profile));
            return 3;
        }
    }

    // Enumerate every ordered 3-slot combination of the 13 raw-ROM templates
    // with the +0x10 ready bit both set and clear. Test all 13 source runner
    // records as the active Body/Willpower recipient for every combination.
    std::array<EntityActionRecord, 26U> party_variants{};
    for (std::size_t profile = 0U; profile < source_profiles.size(); ++profile) {
        party_variants[profile] = source_profiles[profile];
        party_variants[profile].flags_10 = static_cast<std::uint8_t>(
            party_variants[profile].flags_10 | 0x01U);
        party_variants[profile + source_profiles.size()] =
            source_profiles[profile];
        party_variants[profile + source_profiles.size()].flags_10 =
            static_cast<std::uint8_t>(
                party_variants[profile + source_profiles.size()].flags_10 &
                0xFEU);
    }

    std::uint64_t party_combinations = 0U;
    for (std::size_t first = 0U; first < party_variants.size(); ++first) {
        for (std::size_t second = 0U; second < party_variants.size(); ++second) {
            for (std::size_t third = 0U; third < party_variants.size(); ++third) {
                const std::array<EntityActionRecord, kPartySlots> party{{
                    party_variants[first], party_variants[second],
                    party_variants[third]}};
                for (std::size_t active = 0U;
                     active < source_profiles.size(); ++active) {
                    if (!check_strip_pair(
                            party, source_profiles[active],
                            "party=" + std::to_string(first) + "," +
                                std::to_string(second) + "," +
                                std::to_string(third) + " active=" +
                                std::to_string(active))) {
                        return 4;
                    }
                    ++party_combinations;
                }
            }
        }
    }
    constexpr std::uint64_t kExpectedPartyCombinations =
        26ULL * 26ULL * 26ULL * 13ULL;
    if (party_combinations != kExpectedPartyCombinations) {
        fail("party permutation count did not cover all ordered slots");
        return 5;
    }

    // Power Focus is item 0x18, acquired as phase 2 by the source service at
    // 0x22C46. Sweep each inventory position, including slot 7, to verify the
    // source's 6..0 scan boundary against every raw-ROM runner preset.
    std::uint64_t focus_position_cases = 0U;
    std::uint64_t overlapping_focus_cases = 0U;
    EntityActionRecord empty_member{};
    for (std::size_t profile = 0U; profile < source_profiles.size(); ++profile) {
        for (std::size_t slot = 0U; slot < kInventorySlots; ++slot) {
            auto focused = source_profiles[profile];
            focused.flags_10 = static_cast<std::uint8_t>(
                focused.flags_10 | 0x01U);
            focused.item_codes.fill(0U);
            focused.item_phases.fill(0U);
            focused.item_codes[slot] = kPowerFocus;
            focused.item_phases[slot] = 2U;
            const std::array<EntityActionRecord, kPartySlots> party{{
                focused, empty_member, empty_member}};
            if (!check_strip_pair(
                    party, source_profiles[profile],
                    "focus profile=" + std::to_string(profile) +
                        " slot=" + std::to_string(slot))) {
                return 6;
            }
            ++focus_position_cases;
        }

        // If several Power Focus items are present, the routine takes the
        // first match while scanning 6..0. Verify every pair, including slot
        // 7 paired with each eligible slot (where slot 7 must lose).
        for (std::size_t high = 0U; high < kInventorySlots; ++high) {
            for (std::size_t low = high + 1U; low < kInventorySlots; ++low) {
                auto focused = source_profiles[profile];
                focused.flags_10 = static_cast<std::uint8_t>(
                    focused.flags_10 | 0x01U);
                focused.item_codes.fill(0U);
                focused.item_phases.fill(0U);
                focused.item_codes[high] = kPowerFocus;
                focused.item_phases[high] = 1U;
                focused.item_codes[low] = kPowerFocus;
                focused.item_phases[low] = 5U;
                const std::array<EntityActionRecord, kPartySlots> party{{
                    focused, empty_member, empty_member}};
                if (!check_strip_pair(
                        party, source_profiles[profile],
                        "overlapping focus profile=" +
                            std::to_string(profile) + " slots=" +
                            std::to_string(high) + "," +
                            std::to_string(low))) {
                    return 8;
                }
                ++overlapping_focus_cases;
            }
        }
    }

    // Sweep all byte values for Sorcery and item phase on one eligible ROM
    // profile. This checks the source ADD.B wrap and all quarter-rounding
    // boundaries without confusing the byte-domain proof with legal skill or
    // item-rating limits.
    std::uint64_t byte_domain_cases = 0U;
    for (std::uint32_t base = 0U; base <= 0xFFU; ++base) {
        for (std::uint32_t phase = 0U; phase <= 0xFFU; ++phase) {
            auto caster = source_profiles[2U]; // Human Gator Shaman.
            caster.flags_10 = static_cast<std::uint8_t>(caster.flags_10 | 1U);
            caster.item18_base = static_cast<std::uint8_t>(base);
            caster.item_codes.fill(0U);
            caster.item_phases.fill(0U);
            caster.item_codes[6U] = kPowerFocus;
            caster.item_phases[6U] = static_cast<std::uint8_t>(phase);
            const std::array<EntityActionRecord, kPartySlots> party{{
                caster, empty_member, empty_member}};
            if (!check_strip_pair(
                    party, source_profiles[2U],
                    "byte-domain Sorcery=" + std::to_string(base) +
                        " focus=" + std::to_string(phase))) {
                return 7;
            }
            ++byte_domain_cases;
        }
    }

    std::cout << "magic_defense_party_permutations=" << party_combinations
              << " focus_positions=" << focus_position_cases
              << " overlapping_focus_pairs=" << overlapping_focus_cases
              << " byte_domain_cases=" << byte_domain_cases << '\n';
    return 0;
#endif
}
