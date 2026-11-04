#pragma once

#include "shadowrun/core/genesis_bus.hpp"

#include <cstddef>
#include <cstdint>

namespace shadowrun {

constexpr std::size_t kEntitySlotCount = 10;
constexpr std::uint32_t kEntitySlotBase = 0x00FF164C;
constexpr std::size_t kEntitySlotStride = 0x12;
constexpr GenesisAddress kAlternateProjectionRoutine = 0x00015390;

// The 18-byte slots walked by ROM 0x481E. Physical offsets are retained in
// field names because the semantic names of the entity fields are not proven.
struct RawEntitySlot {
    std::uint16_t word_00;
    std::uint16_t word_02;
    std::uint16_t word_04;
    std::uint16_t word_06;
    std::uint16_t word_08;
    std::uint16_t word_0A;
    std::uint16_t word_0C;
    std::uint16_t word_0E;
    std::uint16_t flags_10;
};

static_assert(sizeof(RawEntitySlot) == kEntitySlotStride,
              "Genesis entity slot must remain 18 bytes");

// The three source words consumed by ROM routine 0x5A62. Their final game
// meanings are intentionally left neutral.
struct EntityProjectionInput {
    std::uint16_t word_00;
    std::uint16_t word_04;
    std::uint16_t word_08;
};

// Results written by 0x5A62 at record offsets +0x0A and +0x0C, plus the
// routine's D0 return value. Values are words because the original routine
// wraps all intermediate record arithmetic at 16 bits.
struct EntityProjectionResult {
    std::uint16_t screen_x;
    std::uint16_t screen_y;
    bool visible;
    std::uint16_t return_value;
};

// ROM 0x5A56 uses this alternate result contract when the record longword at
// offset +0x3A equals 0x00015390. The coordinates retain their physical
// meaning; the alternate routine returns screen_x regardless of visibility.
struct AlternateEntityProjectionResult {
    std::uint16_t screen_x;
    std::uint16_t screen_y;
    bool visible;
    std::uint16_t return_value;
};

// Direct translation of the field projection routine at ROM 0x5A62.
EntityProjectionResult project_entity_field(const EntityProjectionInput& input,
                                             std::uint16_t camera_x,
                                             std::uint16_t camera_y);

// Direct translation of the alternate branch in ROM routine 0x5A56. This is
// kept separate because its bounds and return value differ from 0x5A62.
AlternateEntityProjectionResult project_alternate_entity_field(
    const EntityProjectionInput& input,
    std::uint16_t camera_x,
    std::uint16_t camera_y);

// Record-level adapter for the ROM's 0x5A56/0x5A62 selector. It reads the
// physical fields from record_address, writes +0x0A/+0x0C, and returns the
// selected routine's D0.W result. The record is allowed to extend beyond the
// compact 0x12-byte active-slot view because the selector lives at +0x3A.
std::uint16_t project_entity_record(GenesisBus& bus,
                                    GenesisAddress record_address,
                                    std::uint16_t camera_x,
                                    std::uint16_t camera_y);

// Apply the same pure transform to the active slots in a caller-provided
// array. The ROM uses this phase in a ten-slot loop at 0x481E; later sprite
// table dispatch is deliberately not folded into this routine.
std::size_t project_active_entity_slots(RawEntitySlot* slots,
                                        std::size_t slot_capacity,
                                        std::uint16_t camera_x,
                                        std::uint16_t camera_y);

// Convenience adapter for the exact in-place writes made by 0x5A62.
EntityProjectionResult project_entity_slot(RawEntitySlot& slot,
                                            std::uint16_t camera_x,
                                            std::uint16_t camera_y);

} // namespace shadowrun
