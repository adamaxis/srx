#pragma once

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct TitleNoiseEffectState {
    std::uint8_t behavior_flags = 0; // FFF0A1
    std::uint16_t countdown = 0;      // FFE0CA
    std::uint16_t random_word = 0;    // FFE0CE
    std::uint16_t transition_state = 0; // FFE0D6
    std::uint16_t completion_state = 0; // FFE0F4
};

struct TitleNoiseEffectSpriteBuild {
    std::uint32_t source_table_token = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t build_flags = 0x2000U;
    std::uint32_t sprite_data_token = 0;
};

struct TitleNoiseEffectCallbacks {
    std::function<void()> attach_behavior_descriptor_e;
    std::function<void()> refresh_menu_selection_after_clear;
    std::function<std::optional<std::uint16_t>()> advance_source_prng;
    // The source passes each of these inline sprite-record streams directly
    // to build_title_sprite_records; this callback is a host-data boundary,
    // not a pointer-table lookup.
    std::function<std::optional<std::uint32_t>(
        std::uint32_t source_table_token)>
        resolve_sprite_data;
    std::function<void(const TitleNoiseEffectSpriteBuild&)> build_sprite;
};

enum class TitleNoiseEffectAction {
    updated,
    rendered,
    finalized,
    prng_unresolved,
    sprite_data_unresolved,
};

// Typed extraction of the six inline sprite-record streams used by ROM
// 0x1BB6A. Unlike pointer-table callers elsewhere in the title effects, the
// source A2 values at 0x1DAB8/0x1DAC2/0x1DACC/0x1DAD6/0x1DAE0/0x1DAEA
// already point at the records consumed by build_title_sprite_records.
std::optional<std::uint32_t> resolve_title_noise_effect_sprite_data(
    std::uint32_t source_table_token);

// ROM 0x1BB6A: advance the Matrix-noise title effect selected by FFF0A1 bit
// 2. The active branch optionally emits the paired top sprites, then always
// runs the seven-position noise row; the inactive branch arms that flag at
// expiry, otherwise advances the source PRNG and runs the same row. The row
// rotates a local D4 copy through the source carry branches and advances x by
// 0x10 before each build.
TitleNoiseEffectAction advance_title_noise_effect(
    TitleNoiseEffectState& state,
    const TitleNoiseEffectCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
