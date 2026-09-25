#include "shadowrun_ghidra/world_mode_routine_conversion.hpp"

#include <string>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldModeRoutineState state{};
    state.resource_token = 0x4321U;
    state.audio_menu_generation = 3U;
    state.observed_audio_menu_generation = 2U;
    state.field_mode = 1U;
    state.animation_seed = 7U;

    std::vector<std::string> calls;
    int ticks = 0;
    const auto record = [&calls](const char* name) {
        calls.emplace_back(name);
    };

    WorldModeRoutineCallbacks callbacks{};
    callbacks.reset_audio_menu_arrays = [&] { record("audio-reset"); };
    callbacks.copy_audio_state = [&] { record("audio-copy"); };
    callbacks.calculate_menu_delay = [&](WorldModeRoutineState& current) {
        record("delay");
        current.menu_delay = 12U;
    };
    callbacks.convert_audio_setup = [&] { record("audio-setup"); };
    callbacks.prepare_mode_entry = [&] { record("mode-entry"); };
    callbacks.clear_plane_a = [&] { record("plane-a"); };
    callbacks.clear_plane_b = [&] { record("plane-b"); };
    callbacks.reset_upload_state = [&] { record("upload-reset"); };
    callbacks.reset_word_stream_area = [&] { record("word-reset"); };
    callbacks.render_menu_footer = [&] { record("footer"); };
    callbacks.upload_menu_resource_pair = [&] { record("menu-pair"); };
    callbacks.decode_hud_source_stream = [&] { record("hud-source"); };
    callbacks.render_hud_sprite_stream = [&] { record("hud-sprites"); };
    callbacks.render_field_hud = [&] { record("field-hud"); };
    callbacks.initialize_menu_slot_states = [&] { record("slots"); };
    callbacks.refresh_field_hud = [&] { record("refresh-hud"); };
    callbacks.write_four_by_four_transfer_batch = [&] { record("4x4"); };
    callbacks.refresh_field_resource_streams = [&] { record("resources"); };
    callbacks.copy_hud_words = [&] { record("hud-copy"); };
    callbacks.attach_behavior_descriptor = [&] { record("behavior"); };
    callbacks.prepare_menu_hud_sequence = [&] { record("menu-sequence"); };
    callbacks.service_audio = [&] { record("audio"); };
    callbacks.run_menu_hud_tick_loop = [&](WorldModeRoutineState& current) {
        record("tick");
        ++ticks;
        current.exit_requested = ticks == 2;
    };
    callbacks.service_audio_variant = [&] { record("audio-variant"); };
    callbacks.service_audio_command_frame = [&] { record("audio-frame"); };
    callbacks.update_world_mode = [&] { record("world-update"); };
    callbacks.upload_field_asset_setup = [&] { record("final-setup"); };
    callbacks.upload_field_asset_sequence = [&] { record("final-sequence"); };
    callbacks.refresh_field_animation_assets = [&] { record("final-animation"); };

    run_world_mode_routine(state, callbacks);

    if (state.published_resource_token != 0x4321U ||
        state.field_effect_marker != 0xFFU || state.iterations != 2U ||
        state.behavior_start != 0x0120U || state.behavior_end != 0x012FU ||
        ticks != 2 || calls.size() < 20U || calls.front() != "audio-reset" ||
        calls[1] != "audio-copy" || calls[2] != "delay" ||
        calls[3] != "audio-setup" || calls.back() != "mode-entry") {
        return 1;
    }

    std::size_t first_tick = 0U;
    while (first_tick < calls.size() && calls[first_tick] != "tick") {
        ++first_tick;
    }
    if (first_tick == calls.size() || first_tick + 1U >= calls.size() ||
        calls[first_tick + 1U] != "audio-variant") {
        return 2;
    }
    return 0;
}
