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
    std::uint8_t ring_index = 0;       // +0x08, six-slot persistent draw ring
    std::uint8_t animation_phase = 0;  // +0x09, 0..2 sprite phase
    std::uint8_t phase_delay = 0;      // +0x0A
    std::uint8_t lifetime = 0;         // +0x0B, 60-step movement lifetime
    std::int16_t velocity_x = 0;       // +0x0C
    std::int16_t velocity_y = 0;       // +0x0E
    std::int16_t target_x = 0;         // +0x10
    std::int16_t direction = 0;        // +0x12; zero mirrors easing offset
};

struct WorldEffectRecordCallbacks {
    // ROM 0x15E56 easing table. The source masks the computed byte index to
    // 0xFF; the normal 60-step lifetime path therefore uses entries 0..61.
    // When absent, the exact extracted 64-entry host table is used.
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

// Typed extraction of the normal 0x15E56 easing entries used by the active
// world-effect lifetime, including the one post-expiry entry read after the
// byte countdown wraps from zero to 0xFF.
std::optional<std::int16_t> resolve_world_effect_easing(
    std::uint8_t easing_index);

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

// Typed extraction of the three direct source-variant tables at 0x1D134,
// 0x1D11C, and 0x1D128. Variant 0 selects 0x1D134, variant 1 selects
// 0x1D11C, and every other source variant selects 0x1D128.
std::optional<std::uint32_t> resolve_world_effect_sprite_data(
    std::uint8_t source_variant, std::uint8_t animation_phase);

// ROM 0x1D0DE: resolve the source table/phase for the record and convert its
// fixed-point position to host sprite coordinates.
std::optional<WorldEffectSpriteSource> resolve_world_effect_sprite(
    const WorldEffectRecordState& state,
    const WorldEffectSpriteCallbacks& callbacks);

struct WorldEffectSpawnState {
    bool work_record_selected = false; // FFE010 != 0 selects FF15B2
    std::int16_t source_x = 0;          // source record +0x0E
    std::int16_t source_y = 0;          // source record +0x10
    std::int16_t destination_x = 0;     // FF1596
    std::int16_t destination_y = 0;     // FF1598
    std::int16_t offset_x = 0x20;       // caller D5
    std::int16_t offset_y = -0x28;      // caller D6
    WorldEffectRecordState record{};
};

struct WorldEffectSpawnCallbacks {
    // 0x1CBF8 is kept as a host-state boundary; its typed implementation
    // clears the seven work packets when the source record permits it.
    std::function<void()> clear_work_record;
    // ROM 0x530DE is invoked with audio command 0x17. The returned D1 word
    // becomes the record's direction flag.
    std::function<std::int16_t()> play_effect_audio;
};

enum class WorldEffectSpawnAction {
    spawned,
    record_already_active,
};

// ROM 0x1CB6A: choose the normal/work record, initialize the effect audio and
// motion fields, and derive 60-step fixed-point velocity toward the target.
WorldEffectSpawnAction spawn_world_effect_record(
    WorldEffectSpawnState& state,
    const WorldEffectSpawnCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
