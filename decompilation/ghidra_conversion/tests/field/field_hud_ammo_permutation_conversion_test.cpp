#include "shadowrun_ghidra/core/item_catalog_conversion.hpp"
#include "shadowrun_ghidra/field/field_hud_conversion.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#ifndef SHADOWRUN_US_ROM_PATH
#error "SHADOWRUN_US_ROM_PATH must point to the source ROM"
#endif

namespace {

using namespace shadowrun::ghidra_conversion;

std::uint16_t read_be16(const std::vector<std::uint8_t>& rom,
                        const std::size_t offset) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(rom[offset]) << 8U) |
        static_cast<std::uint16_t>(rom[offset + 1U]));
}

std::uint8_t expected_positive_or_zero(const std::uint8_t value,
                                       const std::int8_t delta) {
    const auto wrapped = static_cast<std::int8_t>(
        static_cast<std::uint8_t>(static_cast<int>(value) + delta));
    return wrapped <= 0 ? 0U : static_cast<std::uint8_t>(wrapped);
}

bool fail(const std::string& message) {
    std::cerr << message << '\n';
    return false;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::ifstream input(SHADOWRUN_US_ROM_PATH, std::ios::binary);
    if (!input) {
        return fail("could not open US ROM for HUD counter audit") ? 0 : 1;
    }
    const std::vector<std::uint8_t> rom(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>{});
    constexpr std::size_t kItemTable = 0xD4540U;
    constexpr std::size_t kAmmoDigitTable = 0x6AC46U;
    constexpr std::size_t kAmmoDigitCount = 31U;
    if (rom.size() <= kItemTable + 12U * 0x30U + 0x24U ||
        rom.size() < kAmmoDigitTable + kAmmoDigitCount * 4U) {
        return fail("US ROM is too short for HUD source tables") ? 0 : 1;
    }

    const auto& item_catalog = us_item_catalog();
    for (std::size_t item = 0; item < kFieldHudAmmoTypeCaps.size(); ++item) {
        const auto capacity_offset = kItemTable + item * 0x30U + 0x24U;
        const auto source_capacity = rom[capacity_offset];
        if (source_capacity != kFieldHudAmmoTypeCaps[item] ||
            source_capacity != item_catalog[item].source_combat_factor) {
            return fail("HUD capacity disagrees with ROM/catalog at item " +
                        std::to_string(item + 1U))
                       ? 0
                       : 1;
        }
    }

    for (std::size_t value = 0; value < kAmmoDigitCount; ++value) {
        const auto row = kAmmoDigitTable + value * 4U;
        const std::array<std::uint16_t, 2> source_tiles{
            read_be16(rom, row), read_be16(rom, row + 2U)};
        if (source_tiles != kFieldHudAmmoMeterTiles[value]) {
            return fail("HUD counter tiles disagree with ROM at value " +
                        std::to_string(value))
                       ? 0
                       : 1;
        }
    }

    constexpr std::array<std::int8_t, 5> kDeltas{{-3, -1, 0, 1, 3}};
    constexpr std::array<std::uint16_t, 3> kPartyRecordTypes{{0U, 1U, 2U}};
    std::size_t firearm_cases = 0U;
    for (std::size_t item = 0; item < kFieldHudAmmoTypeCaps.size(); ++item) {
        const auto code = static_cast<std::uint8_t>(item + 1U);
        const auto capacity = kFieldHudAmmoTypeCaps[item];
        for (const auto slot_word : kPartyRecordTypes) {
            for (unsigned ammo = 0; ammo <= 0xFFU; ++ammo) {
                for (const auto delta : kDeltas) {
                    FieldHudAmmoMeterState state{};
                    state.slot_word = slot_word;
                    state.flags_byte = 0x01U;
                    state.equipped_inventory_slot = 0;
                    state.source_type = code;
                    state.ammo = static_cast<std::uint8_t>(ammo);
                    const auto plan = advance_field_hud_ammo_meter(state, delta);
                    const auto positive = expected_positive_or_zero(
                        static_cast<std::uint8_t>(ammo), delta);
                    const auto expected = std::min(positive, capacity);
                    const auto expected_nametable = slot_word == 0U
                        ? 0xF4C4U
                        : slot_word == 1U ? 0xF8C4U : 0xFCC4U;
                    if (state.ammo != expected || plan.displayed != expected ||
                        plan.tiles != kFieldHudAmmoMeterTiles[expected] ||
                        plan.nametable != expected_nametable ||
                        plan.reload_assets) {
                        return fail("firearm counter state mismatch for item " +
                                    std::to_string(code) + ", ammo " +
                                    std::to_string(ammo) + ", delta " +
                                    std::to_string(delta));
                    }
                    ++firearm_cases;
                }
            }
        }
    }

    std::size_t inactive_cases = 0U;
    constexpr std::array<std::uint8_t, 2> kInactiveFlags{{0U, 0xFEU}};
    constexpr std::array<std::int8_t, 2> kUnarmedSlots{{-1, 0x7F}};
    for (std::size_t item = 0; item < kFieldHudAmmoTypeCaps.size(); ++item) {
        for (unsigned ammo = 0; ammo <= 0xFFU; ++ammo) {
            for (const auto slot_word : kPartyRecordTypes) {
                for (const auto flags : kInactiveFlags) {
                    for (const auto equipped_slot : kUnarmedSlots) {
                        FieldHudAmmoMeterState state{};
                        state.slot_word = slot_word;
                        state.flags_byte = flags;
                        state.equipped_inventory_slot = equipped_slot;
                        state.source_type = static_cast<std::uint8_t>(item + 1U);
                        state.ammo = static_cast<std::uint8_t>(ammo);
                        const auto before = state.ammo;
                        const auto plan = advance_field_hud_ammo_meter(state, -1);
                        if (state.ammo != before || plan.displayed != 0U ||
                            plan.tiles != kFieldHudAmmoMeterTiles[0] ||
                            plan.reload_assets) {
                            return fail("inactive/unarmed record exposed ammo")
                                       ? 0
                                       : 1;
                        }
                        ++inactive_cases;
                    }
                }
            }
        }
    }

    std::size_t non_ammunition_items = 0U;
    for (std::uint8_t code = 0x10U; code <= kUsItemDefinitionCount; ++code) {
        for (std::uint8_t slot = 0U; slot < 8U; ++slot) {
            for (unsigned ammo = 0U; ammo <= 0xFFU; ++ammo) {
                FieldHudAmmoMeterState state{};
                state.slot_word = 1U;
                state.flags_byte = 0x01U;
                state.equipped_inventory_slot =
                    static_cast<std::int8_t>(slot);
                state.source_type = code;
                state.ammo = static_cast<std::uint8_t>(ammo);
                state.clip_counts[slot] = 5U;
                state.clip_aux[slot] = code;
                const auto plan = advance_field_hud_ammo_meter(state, -1);
                if (state.ammo != ammo || state.clip_counts[slot] != 5U ||
                    state.clip_aux[slot] != code || plan.displayed != 0U ||
                    plan.tiles != kFieldHudAmmoMeterTiles[0] ||
                    plan.nametable != 0xF8C4U || plan.reload_assets) {
                    return fail("non-ammunition item displayed a counter for "
                                "item " + std::to_string(code));
                }
                ++non_ammunition_items;
            }
        }
    }

    std::size_t firing_service_cases = 0U;
    for (std::uint8_t code = 1U; code <= 12U; ++code) {
        const auto delta = code >= 8U && code < 0x0BU ? -3 : -1;
        for (const auto slot_word : kPartyRecordTypes) {
            for (unsigned ammo = 0U; ammo <= 0xFFU; ++ammo) {
                FieldHudAmmoMeterState state{};
                state.slot_word = slot_word;
                state.flags_byte = 0x01U;
                state.equipped_inventory_slot = 0;
                state.source_type = code;
                state.ammo = static_cast<std::uint8_t>(ammo);
                const auto plan = service_field_hud_ammo(state);
                const auto expected_nametable = slot_word == 0U
                    ? 0xF4C4U
                    : slot_word == 1U ? 0xF8C4U : 0xFCC4U;
                if (ammo == 0U) {
                    if (state.ammo != 0U || plan.displayed != 0U ||
                        plan.tiles != std::array<std::uint16_t, 2>{} ||
                        plan.reload_assets) {
                        return fail("empty firearm reserve changed on service");
                    }
                } else {
                    const auto positive = expected_positive_or_zero(
                        static_cast<std::uint8_t>(ammo), delta);
                    const auto expected = std::min(
                        positive, kFieldHudAmmoTypeCaps[code - 1U]);
                    if (state.ammo != expected || plan.displayed != expected ||
                        plan.tiles != kFieldHudAmmoMeterTiles[expected] ||
                        plan.nametable != expected_nametable ||
                        plan.reload_assets) {
                        return fail("firing service delta mismatch for item " +
                                    std::to_string(code));
                    }
                }
                ++firing_service_cases;
            }
        }
    }

    std::size_t phase_cases = 0U;
    constexpr std::array<std::int8_t, 3> kPhaseDeltas{{-1, 0, 1}};
    for (std::uint8_t code = 0x0DU; code <= 0x0FU; ++code) {
        for (std::uint8_t slot = 0U; slot < 8U; ++slot) {
            for (const auto slot_word : kPartyRecordTypes) {
                for (unsigned count = 0U; count <= 30U; ++count) {
                    for (const auto delta : kPhaseDeltas) {
                        if (count == 30U && delta > 0) {
                            continue; // source digit table ends at 30.
                        }
                        FieldHudAmmoMeterState state{};
                        state.slot_word = slot_word;
                        state.flags_byte = 0x01U;
                        state.equipped_inventory_slot =
                            static_cast<std::int8_t>(slot);
                        state.source_type = code;
                        state.ammo = 1U;
                        state.clip_counts[slot] =
                            static_cast<std::uint8_t>(count);
                        state.clip_aux[slot] = code;
                        const auto expected = expected_positive_or_zero(
                            static_cast<std::uint8_t>(count), delta);
                        const auto plan = advance_field_hud_ammo_meter(state,
                                                                      delta);
                        const auto expected_nametable = slot_word == 0U
                            ? 0xF4C4U
                            : slot_word == 1U ? 0xF8C4U : 0xFCC4U;
                        if (plan.nametable != expected_nametable ||
                            plan.displayed != expected ||
                            plan.tiles != kFieldHudAmmoMeterTiles[expected]) {
                            return fail("inventory counter display mismatch for "
                                        "item " + std::to_string(code) +
                                        ", slot " + std::to_string(slot) +
                                        ", count " + std::to_string(count) +
                                        ", delta " + std::to_string(delta));
                        }
                        if (expected == 0U) {
                            if (state.clip_counts[slot] != 0U ||
                                state.clip_aux[slot] != 0U ||
                                state.equipped_inventory_slot != 0x7F ||
                                state.source_type != 0U ||
                                !plan.reload_assets) {
                                return fail("empty inventory count did not clear "
                                            "the equipped item");
                            }
                        } else if (state.clip_counts[slot] != expected ||
                                   state.clip_aux[slot] != code ||
                                   state.equipped_inventory_slot !=
                                       static_cast<std::int8_t>(slot) ||
                                   state.source_type != code ||
                                   plan.reload_assets) {
                            return fail("positive inventory count changed its "
                                        "equipped item");
                        }
                        ++phase_cases;
                    }
                }
            }
        }
    }

    std::size_t grenade_service_cases = 0U;
    for (std::uint8_t code = 0x0DU; code <= 0x0FU; ++code) {
        for (std::uint8_t slot = 0U; slot < 8U; ++slot) {
            for (unsigned count = 0U; count <= 30U; ++count) {
                FieldHudAmmoMeterState state{};
                state.slot_word = 0U;
                state.flags_byte = 0x01U;
                state.equipped_inventory_slot =
                    static_cast<std::int8_t>(slot);
                state.source_type = code;
                state.ammo = 1U;
                state.clip_counts[slot] = static_cast<std::uint8_t>(count);
                state.clip_aux[slot] = code;
                const auto plan = service_field_hud_ammo(state);
                const auto expected = expected_positive_or_zero(
                    static_cast<std::uint8_t>(count), -1);
                if (state.clip_counts[slot] != expected ||
                    plan.displayed != expected ||
                    plan.tiles != kFieldHudAmmoMeterTiles[expected] ||
                    (expected == 0U &&
                     (state.clip_aux[slot] != 0U ||
                      state.equipped_inventory_slot != 0x7F ||
                      state.source_type != 0U || !plan.reload_assets)) ||
                    (expected != 0U &&
                     (state.clip_aux[slot] != code ||
                      state.equipped_inventory_slot !=
                          static_cast<std::int8_t>(slot) ||
                      state.source_type != code || plan.reload_assets))) {
                    return fail("grenade service did not consume one source "
                                "phase for item " + std::to_string(code));
                }
                ++grenade_service_cases;
            }
        }
    }

    std::cout << "rom_weapon_capacities=12 digit_rows=31 firearm_states="
              << firearm_cases << " inactive_states=" << inactive_cases
              << " non_ammo_item_states=" << non_ammunition_items
              << " firing_service_states=" << firing_service_cases
              << " grenade_phase_states=" << phase_cases
              << " grenade_service_states=" << grenade_service_cases << '\n';
    return 0;
}
