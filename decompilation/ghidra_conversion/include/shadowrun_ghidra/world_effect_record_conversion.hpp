#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

// The 0x14-byte record at FF15B2/FF159E is a host-side motion record.  The
// byte and word fields deliberately retain the source record's mixed layout,
// but callers do not need to expose Genesis addresses or a byte-addressed RAM
// image.
struct WorldEffectRecordState {
    std::int16_t position_x = 0;       // +0x00, 8.8 source position
    std::int16_t position_y = 0;       // +0x02, 8.8 source position
    std::uint8_t frame_delay = 0;      // +0x04
    std::uint8_t source_variant = 0;   // +0x05, 0..2 source table selector
    std::uint8_t active = 0;           // +0x06, source uses 0xFF while active
    std::uint8_t animation_phase = 0;  // +0x09, 0..2 sprite phase
    std::uint8_t phase_delay = 0;      // +0x0A
    std::uint8_t lifetime = 0;         // +0x0B, 60-step movement lifetime
    std::int16_t velocity_x = 0;       // +0x0C
    std::int16_t velocity_y = 0;       // +0x0E
    std::int16_t target_x = 0;         // +0x10
    std::int16_t direction = 0;        // +0x12; zero mirrors easing offset
};

struct WorldEffectRecordCallbacks {
    // ROM 0x15E56 easing table. The index is (0x3C - lifetime) & 0x3F.
    std::function<std::int16_t(std::uint8_t easing_index)> resolve_easing;
};

enum class WorldEffectRecordAction {
    advanced,
    easing_unresolved,
};

// ROM 0x1D048: advance one active world/title effect record, including its
// two timers, easing offset, fixed-point movement, and three-phase source
// animation counter.
WorldEffectRecordAction advance_world_effect_record(
    WorldEffectRecordState& state,
    const WorldEffectRecordCallbacks& callbacks);

struct WorldEffectSpriteSource {
    std::uint32_t source_table_token = 0;
    std::uint16_t position_x = 0;
    std::uint16_t position_y = 0;
};

struct WorldEffectSpriteCallbacks {
    // ROM 0x1D0DE chooses one of 0x1D134/0x1D11C/0x1D128 by source variant
    // and returns the selected phase entry as a host asset token.
    std::function<std::optional<std::uint32_t>(
        std::uint8_t source_variant, std::uint8_t animation_phase)>
        resolve_sprite_source;
};

// ROM 0x1D0DE: resolve the source table/phase for the record and convert its
// fixed-point position to host sprite coordinates.
std::optional<WorldEffectSpriteSource> resolve_world_effect_sprite(
    const WorldEffectRecordState& state,
    const WorldEffectSpriteCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
