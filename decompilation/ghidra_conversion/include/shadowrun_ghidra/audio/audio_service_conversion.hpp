#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct AudioServiceCallbacks {
    std::function<void()> begin_bus_access;
    std::function<void()> flush_audio_command;
    std::function<void()> append_audio_command;
    std::function<void()> end_bus_access;
};

struct AudioServiceTickState {
    std::uint8_t phase = 0;
};

// ROM 0x17D3E: increment the byte-sized audio phase before servicing audio.
bool advance_audio_service_tick(
    AudioServiceTickState& state,
    bool audio_enabled,
    const AudioServiceCallbacks& callbacks);

// ROM 0x52F3C: service the audio command queue in its source order. The
// queue contents and Z80 bus are host-owned boundaries rather than emulated
// memory or processor state.
bool service_audio_command_queue(const AudioServiceCallbacks& callbacks);

// ROM 0x530DE: perform the queue service only when the source audio-enable
// flag is set.
bool service_audio_if_enabled(bool audio_enabled,
                              const AudioServiceCallbacks& callbacks);

struct EntityAudioState {
    std::uint8_t flags = 0;
    std::uint8_t behavior_variant = 0;
    std::uint8_t update_marker = 0;
    std::uint16_t behavior_timer = 0;
    std::uint16_t current_x = 0;
    std::uint16_t current_y = 0;
    std::uint16_t destination_x = 0;
    std::uint16_t destination_y = 0;
    std::uint16_t effect_word = 0;
};

// ROM 0x15354: arm an entity's audio/action record and invoke the audio
// service boundary after copying its current position.
void activate_entity_audio_state(
    EntityAudioState& state,
    const std::function<void()>& service_audio);

struct EntityAudioEffectRecord {
    std::uint8_t flags = 0;          // +0x10
    std::uint8_t flags2 = 0;         // +0x11
    std::int16_t timer = 0;          // +0x16
    std::uint16_t rate = 0;          // +0x18
    std::uint16_t aux = 0;           // +0x1E
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t dest_x = 0;        // +0x2
    std::uint16_t dest_y = 0;        // +0x6
    std::uint8_t phase = 0;          // +0x41
    std::uint16_t jitter_count = 0;  // +0x44
    std::int16_t lifetime = 0;       // +0x46
    std::int16_t bounce = 0;         // +0x40
    std::int16_t lift = 0;           // +0x36
    std::int16_t lift_sum = 0;       // +0x34
    std::int16_t height = 0;         // +0x8
    std::int16_t heading_x = 0;      // +0x4C
    std::int16_t heading_y = 0;      // +0x4E
};

enum class EntityAudioEffectResult {
    waiting,
    jittered,
    retriggered,
    motion,
    expired,
};

// ROM 0x15390: +0x10 bit 7 selects the jitter/expire path; the clear path
// counts +0x46 then +0x40 and either retriggers 0x15354 or continues at
// 0x15426.
EntityAudioEffectResult advance_entity_audio_effect(
    EntityAudioEffectRecord& state,
    std::uint16_t random_word,
    const std::function<void()>& expire,
    const std::function<void()>& retrigger,
    const std::function<void()>& motion);

// ROM 0x15426: ADD 0x20 into +0x36, fold into +0x34, ASR#8 → +0x8. Negative
// height skips the apex. +0x36>=0x40 halves lift and heading; else park.
void apply_spawned_effect_bounce(EntityAudioEffectRecord& state);

enum class SpawnedEffectTerrainResult {
    committed,
    expired,
    deflected,
};

struct SpawnedEffectTerrainWorld {
    bool collision_lock = false;     // FFDFE2
    bool effect_mode_active = false; // FFDF40
    bool skip_deflect = false;       // FFDF68 != 0
};

struct SpawnedEffectTerrainSample {
    std::uint16_t collision = 0;
    std::uint16_t tile = 0;
};

using SpawnedEffectTerrainProbe =
    std::function<SpawnedEffectTerrainSample(std::int32_t x, std::int32_t y)>;

// ROM 0x15488: 0x14F26-style heading<<8 add, height<<4 into D6 low before
// 0xE7C8, then commit / expire / 0x1550E heading deflect.
SpawnedEffectTerrainResult probe_spawned_effect_terrain(
    EntityAudioEffectRecord& state,
    const SpawnedEffectTerrainWorld& world,
    const SpawnedEffectTerrainProbe& probe,
    const std::function<void()>& expire);

} // namespace shadowrun::ghidra_conversion
