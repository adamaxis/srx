#include "shadowrun_ghidra/world_effect_scene_loop_conversion.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    WorldEffectSceneLoopState one_frame{};
    one_frame.source_audio_track = 3U;
    one_frame.footer_blocked = true;
    one_frame.current_runner_footer_visible = true;
    one_frame.title_service_flags = 0x77U;
    one_frame.world_effect_mode = 4U;
    one_frame.title_phase_active = true;
    one_frame.burst_active = true;
    one_frame.noise_effect_active = true;
    one_frame.profile_transition_active = true;
    one_frame.profile_motion_active = true;
    one_frame.title_effect_motion_active = true;
    one_frame.world_effect_motion_active = true;
    one_frame.matrix_transition_active = true;
    one_frame.title_animation_active = true;
    one_frame.persistent_title_effect_active = true;
    one_frame.persistent_world_effect_active = true;

    std::vector<std::string> order;
    WorldEffectSceneLoopCallbacks callbacks{};
    callbacks.reset_word_stream_area = [&] { order.emplace_back("reset"); };
    callbacks.upload_decoded_source_stream =
        [&] { order.emplace_back("upload"); };
    callbacks.initialize_world_effect_mode =
        [&] { order.emplace_back("mode-init"); };
    callbacks.attach_behavior_descriptor_e =
        [&] { order.emplace_back("descriptor"); };
    callbacks.wait_for_vblank = [&] { order.emplace_back("vblank"); };
    callbacks.latch_controller_events = [&] { order.emplace_back("latch"); };
    callbacks.sample_controller_input = [&] {
        order.emplace_back("sample");
        return std::uint16_t{0};
    };
    callbacks.advance_menu_random_glitch =
        [&] { order.emplace_back("glitch"); };
    callbacks.advance_world_menu_service =
        [&] { order.emplace_back("world-menu"); };
    callbacks.advance_title_world_interaction_service =
        [&] { order.emplace_back("title-resource"); };
    callbacks.handle_menu_directional_input =
        [&] { order.emplace_back("input"); };
    callbacks.advance_world_effect_controller =
        [&] {
            order.emplace_back("effect-controller");
            one_frame.world_effect_mode = 4U;
            one_frame.title_phase_active = true;
            one_frame.burst_active = true;
            one_frame.noise_effect_active = true;
            one_frame.profile_transition_active = true;
            one_frame.profile_motion_active = true;
            one_frame.title_effect_motion_active = true;
            one_frame.world_effect_motion_active = true;
            one_frame.matrix_transition_active = true;
            one_frame.title_animation_active = true;
            one_frame.persistent_title_effect_active = true;
            one_frame.persistent_world_effect_active = true;
            one_frame.title_service_flags = 0x77U;
        };
    callbacks.advance_profile_copy_timer =
        [&] { order.emplace_back("profile"); };
    callbacks.append_title_vertical_sprite =
        [&] { order.emplace_back("vertical"); };
    callbacks.advance_title_phase_cycle =
        [&] { order.emplace_back("phase"); };
    callbacks.advance_title_sprite_resource_service =
        [&] { order.emplace_back("resource"); };
    callbacks.advance_title_sprite_layers =
        [&] { order.emplace_back("layers"); };
    callbacks.advance_title_sweep_animation =
        [&] { order.emplace_back("sweep"); };
    callbacks.advance_audio_menu_countdown =
        [&] { order.emplace_back("audio-menu"); };
    callbacks.advance_title_noise_stream =
        [&] { order.emplace_back("noise-stream"); };
    callbacks.advance_title_reveal_transfer_service =
        [&] { order.emplace_back("phase-service"); };
    callbacks.advance_title_secondary_effect_service =
        [&] { order.emplace_back("profile-service"); };
    callbacks.dispatch_title_phase_handler =
        [&] { order.emplace_back("phase-handler"); };
    callbacks.advance_title_burst_animation =
        [&] { order.emplace_back("burst"); };
    callbacks.advance_title_noise_effect =
        [&] { order.emplace_back("noise-effect"); };
    callbacks.advance_title_profile_transition =
        [&] { order.emplace_back("profile-transition"); };
    callbacks.advance_title_profile_motion =
        [&] { order.emplace_back("profile-motion"); };
    callbacks.advance_title_effect_motion =
        [&] { order.emplace_back("title-motion"); };
    callbacks.advance_world_effect_motion =
        [&] { order.emplace_back("world-motion"); };
    callbacks.advance_world_effect_mode_animation =
        [&] { order.emplace_back("mode-animation"); };
    callbacks.advance_title_animation_cycle =
        [&] { order.emplace_back("title-animation"); };
    callbacks.advance_matrix_program_transition =
        [&] { order.emplace_back("matrix"); };
    callbacks.update_persistent_title_effect =
        [&] { order.emplace_back("persistent-title"); };
    callbacks.advance_persistent_world_effect =
        [&] { order.emplace_back("persistent-world"); };
    callbacks.tick_world_effect_animation =
        [&] { order.emplace_back("tick"); };
    callbacks.append_world_sprite_records =
        [&] { order.emplace_back("sprites"); };

    const auto one_frame_result =
        run_world_effect_scene_loop(one_frame, callbacks, 1U);
    if (!expect(one_frame_result.reason ==
                    WorldEffectSceneExitReason::frame_budget_exhausted &&
                one_frame_result.frames_executed == 1U &&
                one_frame.work_x == 0x0120U && one_frame.work_y == 0x00D0U &&
                one_frame.audio_track == 3U && one_frame.frame_presented &&
                one_frame.dirty_flags == 0x02U &&
                order.front() == "reset" && order[1] == "upload" &&
                order[2] == "mode-init" && order[3] == "descriptor" &&
                order[4] == "vblank" && order[5] == "latch" &&
                order[6] == "sample" && order[7] == "glitch" &&
                order[8] == "world-menu" && order[9] == "title-resource" &&
                order[10] == "input" && order[11] == "effect-controller" &&
                order.back() == "sprites")) {
        return 1;
    }

    WorldEffectSceneLoopState fade{};
    fade.footer_blocked = true;
    fade.current_runner_footer_visible = true;
    int fade_calls = 0;
    int finish_calls = 0;
    WorldEffectSceneLoopCallbacks fade_callbacks{};
    fade_callbacks.advance_title_jitter = [&] {
        ++fade_calls;
        return false;
    };
    fade_callbacks.advance_world_effect_controller =
        [&] { fade.scene_complete = true; };
    fade_callbacks.advance_hud_fade_service = [&] { ++fade_calls; };
    fade_callbacks.finish_title_fade = [&] { ++finish_calls; };
    const auto fade_result = run_world_effect_scene_loop(fade, fade_callbacks);
    if (!expect(fade_result.reason ==
                    WorldEffectSceneExitReason::title_fade_complete &&
                fade_result.frames_executed == 1U && fade_calls == 2 &&
                finish_calls == 1 && !fade.scene_complete)) {
        return 2;
    }

    WorldEffectSceneLoopState handoff{};
    handoff.runner_selection_mode = true;
    int world_sequence_initializations = 0;
    int runner_selection_calls = 0;
    std::vector<std::uint16_t> audio_commands;
    WorldEffectSceneLoopCallbacks handoff_callbacks{};
    handoff_callbacks.initialize_world_sequence = [&] {
        ++world_sequence_initializations;
    };
    handoff_callbacks.advance_world_sequence = [] { return true; };
    handoff_callbacks.advance_world_effect_controller =
        [&] { handoff.exit_requested = true; };
    handoff_callbacks.prepare_mode_entry_service = [] {};
    handoff_callbacks.service_audio_command_frame =
        [&](const std::uint16_t command) { audio_commands.push_back(command); };
    handoff_callbacks.run_runner_selection_screen =
        [&] { ++runner_selection_calls; };
    const auto handoff_result =
        run_world_effect_scene_loop(handoff, handoff_callbacks);
    if (!expect(handoff_result.reason ==
                    WorldEffectSceneExitReason::handed_to_mode &&
                handoff_result.frames_executed == 1U &&
                world_sequence_initializations == 1 &&
                runner_selection_calls == 1 &&
                audio_commands == std::vector<std::uint16_t>{0x0005U})) {
        return 3;
    }

    WorldEffectSceneLoopState refresh{};
    refresh.footer_blocked = true;
    refresh.current_runner_footer_visible = true;
    std::vector<std::uint16_t> refresh_audio;
    int refresh_calls = 0;
    WorldEffectSceneLoopCallbacks refresh_callbacks{};
    refresh_callbacks.sample_controller_input = [] {
        return std::uint16_t{0x8000U};
    };
    refresh_callbacks.service_audio_command_frame =
        [&](const std::uint16_t command) { refresh_audio.push_back(command); };
    refresh_callbacks.run_matrix_loop = [] {};
    refresh_callbacks.refresh_field_resource_streams = [&] {
        ++refresh_calls;
        return true;
    };
    refresh_callbacks.prepare_mode_entry_service = [] {};
    refresh_callbacks.reset_word_stream_area = [] {};
    const auto refresh_result =
        run_world_effect_scene_loop(refresh, refresh_callbacks, 1U);
    if (!expect(refresh_result.reason ==
                    WorldEffectSceneExitReason::frame_budget_exhausted &&
                refresh_result.frames_executed == 1U && refresh_calls == 1 &&
                refresh_audio == std::vector<std::uint16_t>{0x0004U,
                                                             0x0003U})) {
        return 4;
    }

    return 0;
}
