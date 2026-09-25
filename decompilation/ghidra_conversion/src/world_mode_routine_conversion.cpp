#include "shadowrun_ghidra/world_mode_routine_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

void run_world_mode_routine(
    WorldModeRoutineState& state,
    const WorldModeRoutineCallbacks& callbacks) {
    state.published_resource_token = state.resource_token;
    state.menu_delay = 10U;
    state.audio_cursor = 0U;
    state.field_effect_marker = 0U;

    if (state.audio_menu_generation != state.observed_audio_menu_generation) {
        call_once(callbacks.reset_audio_menu_arrays);
        state.observed_audio_menu_generation = state.audio_menu_generation;
    }
    call_once(callbacks.copy_audio_state);
    if (callbacks.calculate_menu_delay) {
        callbacks.calculate_menu_delay(state);
    }

    state.exit_requested = false;
    do {
        state.iteration_seed = state.animation_seed;
        state.hud_sprite_cursor = 0U;
        state.audio_cursor = 0U;
        state.input_mode = 1U;
        state.hud_mode = 0x0CU;
        state.behavior_start = 0U;
        state.behavior_end = 0U;
        state.menu_delay = 0x28U;

        call_once(callbacks.convert_audio_setup);
        if (state.field_mode != 0U) {
            state.field_effect_marker = 0xFFU;
        }
        call_once(callbacks.prepare_mode_entry);
        call_once(callbacks.clear_plane_a);
        call_once(callbacks.clear_plane_b);
        call_once(callbacks.reset_upload_state);
        call_once(callbacks.reset_word_stream_area);
        call_once(callbacks.render_menu_footer);
        call_once(callbacks.upload_menu_resource_pair);
        call_once(callbacks.decode_hud_source_stream);
        call_once(callbacks.render_hud_sprite_stream);
        call_once(callbacks.render_hud_sprite_stream);
        call_once(callbacks.render_field_hud);
        call_once(callbacks.initialize_menu_slot_states);
        call_once(callbacks.refresh_field_hud);
        call_once(callbacks.write_four_by_four_transfer_batch);
        call_once(callbacks.refresh_field_resource_streams);
        call_once(callbacks.copy_hud_words);
        call_once(callbacks.attach_behavior_descriptor);
        state.behavior_start = 0x0120U;
        state.behavior_end = 0x012FU;
        call_once(callbacks.prepare_menu_hud_sequence);
        call_once(callbacks.service_audio);
        if (callbacks.run_menu_hud_tick_loop) {
            callbacks.run_menu_hud_tick_loop(state);
        }
        call_once(callbacks.service_audio_variant);
        call_once(callbacks.service_audio_command_frame);
        call_once(callbacks.render_hud_sprite_stream);
        call_once(callbacks.update_world_mode);
        ++state.iterations;
    } while (!state.exit_requested);

    call_once(callbacks.upload_field_asset_setup);
    call_once(callbacks.upload_field_asset_sequence);
    call_once(callbacks.refresh_field_animation_assets);
    call_once(callbacks.prepare_mode_entry);
}

} // namespace shadowrun::ghidra_conversion
