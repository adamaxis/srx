#pragma once

#include "shadowrun_ghidra/runner_roster_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kRunnerRecordInventorySlots = 8U;
constexpr std::size_t kRunnerRecordAttributeCount = 8U;
constexpr std::size_t kRunnerRecordSkillCount = 12U;
constexpr std::size_t kRunnerRecordAuxiliaryCount = 4U;

// The source keeps a live runner in a 0x100-byte record. These are the
// fields already proven by the ROM consumers; bytes not represented here are
// intentionally left opaque rather than assigned invented meanings.
struct RunnerRecordFields {
    std::uint16_t projection_word_08 = 0;
    std::uint16_t kind = 0;
    std::uint16_t flags = 0;
    std::uint32_t state_pointer = 0;
    std::uint16_t state_selector = 0;
    std::uint16_t timing_multiplier = 0;
    std::uint16_t timing_accumulator = 0;
    std::uint16_t timing_threshold = 0;
    std::uint16_t state_index = 0;
    std::uint16_t record_type = 0;

    std::uint8_t race_index = 0;
    std::uint8_t archetype_index = 0;
    std::uint8_t physical_value = 0;
    std::uint8_t mental_value = 0;
    std::int8_t equipped_inventory_slot = -1;
    std::uint8_t equipped_item_code = 0;
    std::uint8_t posture = 0;
    std::uint8_t ammo_rounds = 0;
    std::uint8_t ammo_clips = 0;
    std::uint8_t equipped_armor_index = 0;
    std::uint16_t cyberware_flags = 0;
    // ROM 0xFBA6 indexes the magic/spell table with the big-endian word at
    // record +0xA0.
    std::uint16_t magic_table_index = 0;

    std::array<std::uint8_t, kRunnerRecordInventorySlots>
        inventory_item_codes{};
    std::array<std::uint8_t, kRunnerRecordInventorySlots>
        inventory_phase_bytes{};
    std::array<std::uint8_t, kRunnerRecordAttributeCount> attributes{};
    std::array<std::uint8_t, kRunnerRecordSkillCount> skills{};
    // The source leaves four auxiliary bytes between the displayed values.
    std::array<std::uint8_t, kRunnerRecordAuxiliaryCount> auxiliary{};
};

// ROM consumers:
//   0x044BC initializes the live record from an archetype/template.
//   0x0D82A reads +0x77 and +0x9A/+0x9B for the Attributes/Skills view.
//   0x0D5DC reads +0x90 for installed cyberware.
//   0x0FBA6 reads +0x7F and the big-endian +0xA0 spell-table selector.
//   0x107CE mutates +0x4D (posture).
//   0x11740, 0x11B58, 0x12A8C, and 0x12CEE read equipment, inventory,
//       ammunition, armor, skills, and cyberware to select source branches.
// This is the typed host-side exposure of those source field accesses. It is
// conversion/reference code only; it is not linked into the native runtime.
RunnerRecordFields read_runner_record_fields(const LiveRunnerRecord& record);

// Writes only fields with a confirmed source meaning. Opaque bytes in the
// destination remain untouched, preserving the source record instead of
// silently inventing a complete schema.
void write_runner_record_fields(const RunnerRecordFields& fields,
                                LiveRunnerRecord& destination);

} // namespace shadowrun::ghidra_conversion
