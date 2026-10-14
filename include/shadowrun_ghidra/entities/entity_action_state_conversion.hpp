#pragma once

#include "shadowrun_ghidra/entities/entity_state_primitives_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct EntityActionState {
    std::uint8_t action_code = 0;   // +0x57
    std::uint8_t state_code = 0;    // +0x56
    std::uint8_t active_marker = 0; // +0x4E
    std::uint8_t reset_marker = 0;  // +0x9D
    std::uint16_t scaled_ammo = 0;  // D1 after 0x36FC
};

// ROM 0x13466 MOVE.B (0x7F).w,+0x56. Byte at ROM 0x7F is 0x20 (IRQ7 vector).
constexpr std::uint8_t kUnequippedActionType = 0x20U;

// ROM 0x13466: action 0 writes +0x56=kUnequippedActionType and clears
// +0x57/+0x9D. Nonzero copies D7 to +0x56; if +0x4E!=0, scale +0x4E by
// D4540+(action-1)*0x30+0x24 over 100 with rem>=0x32 round-up, then 0x36FC
// and 0x3404.
void set_entity_action_state(
    EntityActionState& state,
    std::uint8_t action_code,
    std::uint8_t nonzero_state_code,
    const std::function<void()>& service_action,
    const std::function<void()>& prepare_assets);

struct EntityActionCandidateDispatchState {
    std::uint8_t service_marker = 0;
    std::uint8_t action_flags = 0;
    bool alternate_mode = false;
};

// ROM 0x13874: clear the service marker and dispatch candidate collection
// according to the source mode/low-six-bit action flags.
void dispatch_entity_action_candidates(
    EntityActionCandidateDispatchState& state,
    const std::function<void()>& dispatch_primary,
    const std::function<void()>& dispatch_secondary);

// ROM 0x13AE6: BSR 0x13AF2 then JSR 0x13466. D0 is the first +0x66 byte < 0xD
// (or 0); D7 is the skip count written to +0x56.
void advance_entity_action_threshold(
    EntityActionState& state,
    const std::array<std::uint8_t, 8>& action_values,
    const std::function<void()>& service_action,
    const std::function<void()>& prepare_assets);

struct EntityActionLaneAvailability {
    std::uint8_t primary_occupancy = 0;   // FF014C
    std::uint8_t secondary_occupancy = 0; // FF024C
    std::uint8_t tertiary_occupancy = 0;  // FF034C
    bool secondary_marker_negative = false; // FFFC0C BMI skips lane 1
    bool tertiary_marker_negative = false;  // FFFC0D BMI fails
};

struct EntityActionLaneResult {
    std::uint8_t lane = 0; // D0; FFE190 when available
    bool available = false; // ORI/ANDI CCR carry
};

// ROM 0x13C20: first slot whose unsigned +0x4C < 4. FFFC0C BMI skips lane 1;
// FFFC0D BMI returns carry clear with D0 still 1.
EntityActionLaneResult select_entity_action_lane(
    const EntityActionLaneAvailability& availability);

struct EntityActionSourceRecord {
    std::uint8_t code = 0;
    std::uint8_t value = 0;
};

struct EntityActionRecordTable {
    std::vector<EntityActionSourceRecord> selected;
    std::uint8_t special_value = 0;
    bool terminated = false;
};

// ROM 0x13C74: retain only source action codes 0/1/7/8, publish code 11's
// side value, and close the host table with the source sentinel.
void collect_entity_action_records(
    const std::vector<EntityActionSourceRecord>& source,
    EntityActionRecordTable& destination);

} // namespace shadowrun::ghidra_conversion
