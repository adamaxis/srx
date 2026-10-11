#pragma once

#include "shadowrun_ghidra/session/runner_roster_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kRunnerRecordInventorySlots = 8U;
constexpr std::size_t kRunnerRecordAttributeCount = 8U;
constexpr std::size_t kRunnerRecordSkillCount = 12U;
constexpr std::size_t kRunnerRecordAuxiliaryCount = 4U;

// ROM 0x1E69C reads the first live runner's +0x8C skill byte when it
// calculates a Johnson offer's negotiation roll. This is the last entry in
// the source Attributes/Skills skill sequence at ROM 0xC1A2A.
enum class RunnerSkillIndex : std::uint8_t {
    sorcery = 0U,
    firearms = 1U,
    pistols = 2U,
    smgs = 3U,
    shotguns = 4U,
    melee_combat = 5U,
    throwing = 6U,
    computer = 7U,
    biotech = 8U,
    electronics = 9U,
    reputation = 10U,
    negotiation = 11U,
};

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
    // ROM 0x1253E writes the First Aid continuation at record +0x3A.
    std::uint32_t first_aid_continuation = 0;
    // ROM 0x110BC reads active record +0x2C as the command-overview panel
    // parameter. This is a presentation field, not an address or a VRAM
    // handle, so expose it with the other confirmed live-record values.
    std::uint16_t command_overview_panel_parameter = 0;
    std::uint16_t record_type = 0;
    // ROM 0x3404 reads +0x50/+0x54 on the live record to select the current
    // eight-column face table and its byte offset before uploading the 4x4.
    std::uint32_t hud_portrait_table_pointer = 0;
    std::uint16_t hud_portrait_table_offset = 0;

    std::uint8_t race_index = 0;
    // Source +0x9A. The Attributes/Skills upgrade table at ROM 0xC22A6
    // indexes this species/race value separately from the record header byte
    // at +0x00.
    std::uint8_t species_index = 0;
    std::uint8_t archetype_index = 0;
    // ROM 0x36FC/0xEF08 use +0x4C as the entity delay meter. It is also the
    // value classified into the Hotel's medical-attention/recovery branches.
    std::uint8_t delay_meter = 0;
    // ROM 0x37EC/0xEF08 use +0x9C as the entity behavior meter.
    std::uint8_t behavior_meter = 0;
    // Source +0x9D. Magic force selection publishes the selected force here
    // before the target/effect path begins.
    std::uint8_t magic_force = 0;
    // Source +0x9F. Action-result routines publish thresholded successes or
    // -1 here after the compact-runner timing gate.
    std::int8_t action_result = -1;
    // ROM 0xA8EA decrements this signed frame gate at +0xDC for active
    // entities; 0xAAD2 waits for it to become negative before player action.
    std::int16_t animation_action_gate = 0;
    std::int8_t equipped_inventory_slot = -1;
    std::uint8_t equipped_item_code = 0;
    std::uint8_t posture = 0;
    // In the weapon/inventory path, the source reuses +0x4E: it is loaded
    // rounds while a clip-using weapon is equipped and the loaded percentage
    // while no weapon is active or non-weapon gear is equipped. Weapon changes
    // normalize through that percentage before applying the new weapon's
    // +0x24 limit. Other action paths may reuse the same source byte.
    std::uint8_t ammo_rounds = 0;
    // Source +0x4F is the runner's shared Universal Gun Clips count, not a
    // per-inventory-item clip pool.
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
    // Auxiliary source bytes remain positional unless a caller has a proven
    // meaning. ROM 0x59C34 adds the story-service Karma reward to index 2,
    // which is the live-record byte at +0x8D.
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

// The world-effect animation routines use a different, compact roster than
// the 0x100-byte live runner record above. ROM 0x1A4EA and 0x1A538 select
// FF1100 + (FFF118 * 0x14), then read +0x10/+0x0F respectively.
struct CurrentRunnerTimingRecord {
    // ROM 0x1A538 tests compact record +0x0F as a nonzero marker.
    bool active_effect = false;
    // ROM 0x1A4EA indexes the signed animation-delta table with compact
    // record +0x10. This is a logical table index, not a memory offset.
    std::uint8_t animation_index = 0;
};

struct RunnerTimingRosterState {
    std::vector<CurrentRunnerTimingRecord> records;
    std::size_t current_record_index = 0;
};

// ROM 0x1A4EA: use the selected compact record's animation index into the
// 21-entry signed timing table recovered from ROM 0x1A50E..0x1A536. Invalid
// host indexes and an out-of-range current record are explicit no-results.
std::optional<std::int16_t> lookup_runner_animation_delta(
    const RunnerTimingRosterState& roster);

// ROM 0x1A538: test the selected compact record's +0x0F effect marker.
bool runner_record_has_active_effect(const RunnerTimingRosterState& roster);

} // namespace shadowrun::ghidra_conversion
