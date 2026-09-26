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
    // Resolve one source sprite pointer and its +8 payload.
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

// ROM 0x1BB6A: advance the title noise/effect branch selected by FFF0A1 bit
// 2. The active branch emits paired sprites after a seven-step countdown;
// the inactive branch arms that flag at expiry, otherwise advances the source
// PRNG and emits seven sprites selected by two rotated low bits per pass.
TitleNoiseEffectAction advance_title_noise_effect(
    TitleNoiseEffectState& state,
    const TitleNoiseEffectCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
