#pragma once

#include "shadowrun_ghidra/title_intro/title_effect_animation_batch_conversion.hpp"

#include <cstdint>
#include <functional>
#include <optional>

namespace shadowrun::ghidra_conversion {

struct WorldEffectModeAnimationState {
    std::uint16_t mode = 0; // FFE076
    std::int16_t frame_countdown = 0; // FFE07C
    std::uint16_t position_x = 0; // FFE078
    std::uint16_t position_y = 0; // FFE07A
    std::uint16_t render_flags = 0; // FFDF88
    std::int16_t intro_result = 0; // FFE07E
    std::int16_t audio_transition_result = 0; // FFE0FC
    std::uint16_t transition_state = 0; // FFE0D6
    bool title_mode_visible = false; // FFF0D2 != 0
    std::uint16_t refresh_countdown = 0; // FFE0F0
    bool records_pending = false; // FFE0EE
};

struct WorldEffectModeSpriteBuild {
    std::uint32_t source_table_token = 0x000F1AEEU;
    std::uint8_t table_index = 0;
    std::uint16_t sprite_key = 0x06E0U;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t build_flags = 0x2000U;
    std::uint32_t sprite_data_token = 0;
};

struct WorldEffectModeBehaviorPair {
    std::uint32_t draw_descriptor = 0;
    std::uint32_t update_descriptor = 0;
};

struct WorldEffectModeAnimationCallbacks {
    // The source selects entries in the 0xF1AEE table and passes each entry's
    // +8 payload to the sprite builder. The host resolves that named asset
    // entry by table index instead of exposing a ROM pointer.
    std::function<std::optional<std::uint32_t>(
        std::uint8_t table_index)>
        resolve_sprite_data;
    std::function<void(std::uint16_t sprite_key)> append_sprite_record;
    std::function<void(const WorldEffectModeSpriteBuild&)> build_sprite;

    // ROM 0x562EC updates FFE07E. The returned value is that source field,
    // not a guessed result from the decompiler's D-register output.
    std::function<std::optional<std::int16_t>()> advance_intro_state;
    // ROM 0x530EA with D0=0x18; the returned value is the source D1 word
    // saved into FFE0FC for the following mode-3 branch.
    std::function<std::optional<std::int16_t>(
        std::uint16_t command)> service_audio_frame;
    // ROM 0x1A570.  The callback receives each fixed source-table/origin
    // request in source order, once for 0xF1B1E and once for 0xF1B28.
    std::function<void(const TitleSpriteBatchBuildRequest&)>
        build_title_sprite_batch;
    // ROM 0x5668C, reached from mode 6 at 0x1A770.
    std::function<void()> advance_menu_selection_service_and_refresh;

    std::function<void()> attach_behavior_descriptor_d;
    std::function<void()> attach_behavior_descriptor_e;
    // ROM 0x1B7D0 followed by direct writes of 0x1C2A8/0x1C2AE in mode 5.
    std::function<void()> initialize_behavior_record;
    std::function<void(const WorldEffectModeBehaviorPair&)>
        attach_mode_five_behavior_pair;
};

enum class WorldEffectModeAnimationAction {
    inactive,
    waiting,
    sprite_rendered,
    sprite_data_unresolved,
    intro_state_unresolved,
    audio_result_unresolved,
    title_sprites_rebuilt,
    descriptor_d_attached,
    inline_behavior_initialized,
    descriptor_e_attached,
    mode_finished,
};

// ROM 0x1A5A0: advance the six world-effect animation modes. Position deltas,
// table selection, same-frame mode fallthrough, title refresh flags, and
// behavior-descriptor timing are retained as typed host state; sprite, audio,
// and behavior installation remain explicit boundaries.
WorldEffectModeAnimationAction advance_world_effect_mode_animation(
    WorldEffectModeAnimationState& state,
    const WorldEffectModeAnimationCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
