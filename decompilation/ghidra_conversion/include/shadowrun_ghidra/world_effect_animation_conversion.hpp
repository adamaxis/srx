#pragma once

#include "shadowrun_ghidra/world_effect_mode_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct WorldEffectAnimationState {
    bool animation_active = false;        // FF164A
    bool descriptor_loading = false;      // FFE0F2
    bool transition_active = false;       // FFE082
    bool ffdf8c_nonzero = false;          // FFDF8C
    std::int16_t selected_mode = 0;       // FFE0BC
    WorldEffectWorkRecord work{};         // FF163E
};

struct WorldEffectAnimationSpriteBuild {
    bool transition_table = false;
    std::uint32_t source_table_token = 0;
    std::uint8_t phase = 0;
    std::uint16_t sprite_key = 0x0694U;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t build_flags = 0x2000U;
    std::uint32_t sprite_data_token = 0;
};

struct WorldEffectAnimationCallbacks {
    // ROM 0x1A4EA returns the signed timing delta for the currently selected
    // compact timing-roster record. The current-record lookup is a separate
    // record hill.
    std::function<std::optional<std::int16_t>()> read_runner_animation_delta;
    // Resolve the selected source table entry and its +8 sprite payload as a
    // host token. The transition table is ROM 0xF1C46; normal tables come from
    // the mode descriptor's source-table token.
    std::function<std::optional<std::uint32_t>(
        bool transition_table, std::uint32_t source_table_token,
        std::uint8_t phase)>
        resolve_sprite_data;
    std::function<void(std::uint16_t sprite_key)> append_sprite_record;
    std::function<void(const WorldEffectAnimationSpriteBuild&)> build_sprite;
    std::function<void()> reset_effect_entity;
    std::function<void()> advance_primary_overlay;
    std::function<void()> advance_secondary_overlay;
    std::function<void()> advance_background_overlay;
};

enum class WorldEffectAnimationAction {
    inactive,
    rendered,
    effect_entity_reset,
    animation_delta_unresolved,
    sprite_data_unresolved,
};

// ROM 0x1AA40: advance the descriptor animation, build its current sprite,
// and then update the three secondary overlays. Loading/transition mode uses
// the fixed ROM 0xF1C46 table; the normal mode uses the selected descriptor's
// table token. This is conversion/reference code only.
WorldEffectAnimationAction tick_world_effect_animation(
    WorldEffectAnimationState& state,
    const WorldEffectAnimationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
