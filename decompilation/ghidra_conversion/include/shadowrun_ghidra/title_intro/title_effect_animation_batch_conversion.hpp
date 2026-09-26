#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <optional>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct MenuEntryCallbacks {
    std::function<std::uint8_t()> select_hud_mode;
    std::function<void()> service_entry;
    std::function<std::uint8_t()> read_entry_value;
    std::function<void()> set_selection;
    std::function<void()> render_text;
    std::function<void()> prepare_offset;
};

// ROM 0x1A030: choose the title/menu branch and return its source result code.
std::int16_t select_menu_entry(std::int8_t entry_state,
                               const MenuEntryCallbacks& callbacks);

void refresh_menu_selection_after_clear(
    bool has_pending_value,
    const std::function<void()>& clear_selected,
    const std::function<void()>& set_selection,
    const std::function<void()>& render_text);

void refresh_menu_selection_or_prepare(
    bool has_pending_value,
    const std::function<void()>& prepare_offset,
    const std::function<void()>& clear_selected,
    const std::function<void()>& set_selection,
    const std::function<void()>& render_text);

struct AudioFrameState {
    std::uint16_t field_a = 0;
    std::uint16_t field_b = 0;
    std::uint16_t field_c = 0;
};

void initialize_audio_frame(AudioFrameState& state,
                            const std::function<void()>& service_audio);

struct TitleAnimationCycleState {
    std::int16_t phase = 0;
    std::int16_t countdown = 0;
    bool active = true;
};

bool advance_title_animation_cycle(
    TitleAnimationCycleState& state,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprites);

void build_title_sprites_twice(const std::function<void()>& build_sprites);

struct TitleVerticalBuildState {
    std::uint16_t y = 0;
};

bool append_title_vertical_sprite(
    TitleVerticalBuildState& state,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprites);

struct WorldEffectSeedState {
    std::uint16_t source_x = 0;
    std::uint16_t source_y = 0;
    std::int16_t horizontal_velocity = 0;
    std::int16_t lifetime = 0;
    std::int16_t palette = 0;
    std::int16_t variant = 0;
    bool active = false;
};

void initialize_world_effect_seed(
    WorldEffectSeedState& state,
    std::int16_t variant,
    std::uint16_t source_y,
    const std::function<void()>& build_effect,
    const std::function<void()>& service_audio);

struct TitleTransitionState {
    std::uint16_t position = 0;
    std::uint16_t timer = 0;
    std::uint16_t phase = 0;
    bool dirty = false;
};

void reset_title_transition(TitleTransitionState& state);

struct TitlePhaseCycleState {
    std::int16_t phase = 0;
    std::int16_t countdown = 0;
};

void advance_title_phase_cycle(
    TitlePhaseCycleState& state,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprites);

struct TitleJitterState {
    std::uint16_t frame = 0;
    std::array<std::int16_t, 4> bounds{};
    std::uint32_t dirty_flags = 0;
};

bool advance_title_jitter(
    TitleJitterState& state,
    const std::function<std::uint16_t()>& next_random_word,
    const std::function<void()>& write_word_stream,
    const std::function<void()>& reset_word_stream);

bool dispatch_title_phase_handler(
    std::size_t phase,
    const std::vector<std::function<void()>>& handlers,
    const std::function<void()>& build_sprites);

void copy_title_profile(const std::array<std::uint16_t, 7>& source,
                        std::array<std::uint16_t, 7>& destination,
                        std::uint32_t& dirty_flags);

struct AlternatingProfileState {
    std::int16_t countdown = 0;
    std::array<std::uint16_t, 7> even_profile{};
    std::array<std::uint16_t, 7> odd_profile{};
    std::array<std::uint16_t, 7> destination{};
    std::uint32_t dirty_flags = 0;
};

bool advance_alternating_profile(
    AlternatingProfileState& state,
    const std::function<void()>& final_service);

struct AudioMenuCountdownState {
    std::int16_t frame = 0;
    std::int16_t reload = 0;
    std::uint8_t remaining = 0;
    std::uint8_t entity_flags = 0;
};

bool advance_audio_menu_countdown(
    AudioMenuCountdownState& state,
    const std::function<void()>& service_audio,
    const std::function<void()>& clear_ready_menu_slot);

void initialize_effect_motion_seed(
    WorldEffectSeedState& state,
    bool alternate_direction,
    const std::function<void()>& service_audio);

struct InteractionState {
    std::uint8_t source_flags = 0;
    std::uint8_t latch = 0;
    std::uint8_t phase = 0;
    std::uint16_t countdown = 0;
    std::int16_t x = 0;
    std::int16_t y = 0;
    std::int16_t velocity_x = 0;
    std::int16_t velocity_y = 0;
    bool motion_active = false;
};

struct InteractionCallbacks {
    std::function<void()> initialize_secondary;
    std::function<void()> initialize_primary;
    std::function<void()> advance_motion;
    std::function<void()> service_audio;
};

void update_interaction_state(InteractionState& state,
                              bool source_bit_40,
                              bool global_ready,
                              std::int8_t mode_marker,
                              const InteractionCallbacks& callbacks);

void update_persistent_title_effect(
    bool source_active,
    std::uint8_t slot_flags,
    std::uint16_t& transition_phase,
    const std::function<void()>& prepare_effect,
    const std::function<void()>& advance_effect,
    const std::function<void()>& build_sprites,
    const std::function<void()>& reset_transition,
    const std::function<void()>& refresh_menu);

void advance_title_effect_position(
    std::array<std::int16_t, 4>& positions,
    const std::function<void()>& build_sprites);

void build_title_sprites(const std::function<void()>& build_sprites);

} // namespace shadowrun::ghidra_conversion
