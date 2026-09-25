#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct WorldModeRoutineState {
    std::uint16_t resource_token = 0;
    std::uint16_t published_resource_token = 0;
    std::uint8_t audio_menu_generation = 0;
    std::uint8_t observed_audio_menu_generation = 0;
    std::uint8_t field_mode = 0;
    std::uint8_t animation_seed = 0;
    std::uint8_t iteration_seed = 0;
    std::uint8_t audio_cursor = 0;
    std::uint8_t input_mode = 0;
    std::uint8_t hud_mode = 0;
    std::uint8_t hud_sprite_cursor = 0;
    std::uint16_t menu_delay = 0;
    std::uint16_t behavior_start = 0;
    std::uint16_t behavior_end = 0;
    std::uint8_t field_effect_marker = 0;
    bool exit_requested = false;
    std::uint32_t iterations = 0;
};

struct WorldModeRoutineCallbacks {
    std::function<void()> reset_audio_menu_arrays;
    std::function<void()> copy_audio_state;
    std::function<void(WorldModeRoutineState&)> calculate_menu_delay;
    std::function<void()> convert_audio_setup;
    std::function<void()> prepare_mode_entry;
    std::function<void()> clear_plane_a;
    std::function<void()> clear_plane_b;
    std::function<void()> reset_upload_state;
    std::function<void()> reset_word_stream_area;
    std::function<void()> render_menu_footer;
    std::function<void()> upload_menu_resource_pair;
    std::function<void()> decode_hud_source_stream;
    std::function<void()> render_hud_sprite_stream;
    std::function<void()> render_field_hud;
    std::function<void()> initialize_menu_slot_states;
    std::function<void()> refresh_field_hud;
    std::function<void()> write_four_by_four_transfer_batch;
    std::function<void()> refresh_field_resource_streams;
    std::function<void()> copy_hud_words;
    std::function<void()> attach_behavior_descriptor;
    std::function<void()> prepare_menu_hud_sequence;
    std::function<void()> service_audio;
    std::function<void(WorldModeRoutineState&)> run_menu_hud_tick_loop;
    std::function<void()> service_audio_variant;
    std::function<void()> service_audio_command_frame;
    std::function<void()> update_world_mode;
    std::function<void()> upload_field_asset_setup;
    std::function<void()> upload_field_asset_sequence;
    std::function<void()> refresh_field_animation_assets;
};

// ROM 0x1745E: initialize and run the field/menu mode, preserving the source
// call order while expressing hardware effects as host callbacks and typed
// state. The routine exits only after the mode tick marks the request.
void run_world_mode_routine(WorldModeRoutineState& state,
                            const WorldModeRoutineCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
