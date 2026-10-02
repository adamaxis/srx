#include "shadowrun_ghidra/integration/field_session_bootstrap_conversion.hpp"

#include "shadowrun_ghidra/audio/audio_service_batch_conversion.hpp"
#include "shadowrun_ghidra/field/scene_profile_conversion.hpp"

namespace shadowrun::ghidra_conversion::integration {
namespace {

void set_phase(FieldSessionBootstrapState& state,
               const FieldSessionBootstrapCallbacks& callbacks,
               const FieldSessionBootstrapPhase phase) {
    state.phase = phase;
    state.phase_history.push_back(phase);
    if (callbacks.phase_changed) {
        callbacks.phase_changed(phase);
    }
}

bool has_audio_callbacks(const AudioModeEntryCallbacks& callbacks) {
    return static_cast<bool>(callbacks.run_fixed_audio_transaction) &&
           static_cast<bool>(callbacks.wait_for_vblank_frames) &&
           static_cast<bool>(callbacks.flush_audio_bus) &&
           static_cast<bool>(callbacks.service_audio_tick);
}

bool has_field_resource_callbacks(
    const FieldResourcePreparationCallbacks& callbacks) {
    return static_cast<bool>(callbacks.prepare_asset_setup) &&
           static_cast<bool>(callbacks.upload_asset_sequence) &&
           static_cast<bool>(callbacks.refresh_animation_assets) &&
           static_cast<bool>(callbacks.load_font);
}

bool has_field_viewport_callbacks(
    const FieldViewportPreparationCallbacks& callbacks) {
    return static_cast<bool>(callbacks.prepare_title_resources) &&
           static_cast<bool>(callbacks.copy_mode_selection_state_table) &&
           static_cast<bool>(callbacks.upload_mode_word_block) &&
           static_cast<bool>(callbacks.render_normal_viewport) &&
           static_cast<bool>(callbacks.render_alternate_viewport);
}

bool has_entity_render_callbacks(const EntityRenderPassCallbacks& callbacks) {
    return static_cast<bool>(callbacks.project_screen_position) &&
           static_cast<bool>(callbacks.advance_animation) &&
           static_cast<bool>(callbacks.finish_entity) &&
           static_cast<bool>(callbacks.finish_pass);
}

bool has_required_callbacks(const FieldSessionBootstrapCallbacks& callbacks) {
    return has_audio_callbacks(callbacks.mode_entry_audio) &&
           static_cast<bool>(callbacks.initialize_entity_order) &&
           static_cast<bool>(callbacks.prepare_field_scene_runtime) &&
           has_field_resource_callbacks(callbacks.field_resources) &&
           static_cast<bool>(callbacks.run_world_frame_services) &&
           static_cast<bool>(callbacks.apply_scene_profile_selection) &&
           has_field_viewport_callbacks(callbacks.field_viewports) &&
           static_cast<bool>(callbacks.upload_scene_profile_block) &&
           static_cast<bool>(callbacks.guard_world_effect_player_spawn) &&
           entity_behavior_frame_callbacks_complete(
               callbacks.entity_behavior_frames) &&
           static_cast<bool>(callbacks.update_field_camera) &&
           has_entity_render_callbacks(callbacks.entity_render_pass) &&
           static_cast<bool>(callbacks.wait_for_vblank) &&
           static_cast<bool>(callbacks.advance_title_palette_fade) &&
           static_cast<bool>(callbacks.sample_controller_input);
}

FieldSessionBootstrapResult fail(
    FieldSessionBootstrapState& state,
    const FieldSessionBootstrapCallbacks& callbacks,
    const FieldSessionBootstrapResult result) {
    set_phase(state, callbacks, FieldSessionBootstrapPhase::failed);
    return result;
}

} // namespace

const char* field_session_bootstrap_phase_name(
    const FieldSessionBootstrapPhase phase) {
    switch (phase) {
    case FieldSessionBootstrapPhase::not_started:
        return "not_started";
    case FieldSessionBootstrapPhase::mode_entry_audio:
        return "mode_entry_audio";
    case FieldSessionBootstrapPhase::field_scene_entry:
        return "field_scene_entry";
    case FieldSessionBootstrapPhase::warmup:
        return "warmup";
    case FieldSessionBootstrapPhase::field_ready:
        return "field_ready";
    case FieldSessionBootstrapPhase::failed:
        return "failed";
    }
    return "unknown";
}

FieldSessionBootstrapResult bootstrap_field_session(
    const FieldSessionBootstrapInput& input,
    FieldSessionBootstrapState& state,
    const FieldSessionBootstrapCallbacks& callbacks) {
    state = FieldSessionBootstrapState{};

    constexpr auto action_profile_source_size =
        kEntityActionProfileCount * kEntityActionProfileRowSize;
    if (input.action_selection_profile_source.size() <
            action_profile_source_size ||
        input.field_mode_selection_state_source.size() <
            kFieldModeSelectionStateWordCount ||
        input.field_scene.terrain_stream.empty() ||
        input.field_scene.layer_a.size() < kFieldTerrainLayerWordCount ||
        input.field_scene.layer_b.size() < kFieldTerrainLayerWordCount ||
        !input.field_scene.source_metadata.available ||
        input.field_scene.source_metadata.terrain_collision_rectangles.empty()) {
        return fail(state, callbacks,
                    FieldSessionBootstrapResult::source_data_missing);
    }
    if (!has_required_callbacks(callbacks)) {
        return fail(state, callbacks,
                    FieldSessionBootstrapResult::callback_unresolved);
    }

    set_phase(state, callbacks, FieldSessionBootstrapPhase::mode_entry_audio);
    prepare_audio_mode_entry(callbacks.mode_entry_audio);
    state.audio_mode_entry_complete = true;
    reset_audio_menu_arrays(state.audio_menu_first, state.audio_menu_second);

    // ROM 0x0084E/0x00858 select field mode 2 before entering 0x03074.
    state.mode = FieldSessionMode::field;
    state.field_resource_mode =
        static_cast<std::uint8_t>(FieldSessionMode::field);
    initialize_entity_runtime(state.entity_runtime,
                               callbacks.initialize_entity_order);
    reset_world_effect_globals(state.world_effect_globals);

    if (!copy_action_selection_profile_table(
            input.action_selection_profile_source,
            state.action_selection_profiles) ||
        !copy_field_mode_selection_state(
            input.field_mode_selection_state_source,
            state.field_mode_selection_state)) {
        return fail(state, callbacks,
                    FieldSessionBootstrapResult::source_data_missing);
    }

    set_phase(state, callbacks,
              FieldSessionBootstrapPhase::field_scene_entry);
    FieldScenePreparationCallbacks scene_preparation_callbacks{};
    scene_preparation_callbacks.prepare_field_scene_runtime =
        callbacks.prepare_field_scene_runtime;

    FieldSceneEntryCallbacks scene_entry_callbacks{};
    scene_entry_callbacks.prepare_field_resources = [&callbacks] {
        prepare_field_resources(callbacks.field_resources);
    };
    scene_entry_callbacks.prepare_field_scene_buffers =
        [&state, &input, &scene_preparation_callbacks] {
            state.scene_buffers_prepared = prepare_field_scene_buffers(
                state.field_scene_buffers, input.field_scene,
                scene_preparation_callbacks);
        };
    scene_entry_callbacks.run_world_frame_services =
        callbacks.run_world_frame_services;
    scene_entry_callbacks.apply_scene_profile_selection =
        callbacks.apply_scene_profile_selection;
    scene_entry_callbacks.prepare_field_viewports = [&callbacks, &state] {
        prepare_field_viewports(
            FieldViewportPreparationState{state.field_resource_mode},
            callbacks.field_viewports);
    };
    scene_entry_callbacks.upload_scene_profile_block =
        callbacks.upload_scene_profile_block;
    scene_entry_callbacks.guard_world_effect_player_spawn =
        callbacks.guard_world_effect_player_spawn;

    initialize_field_scene_entry(state.field_scene_entry,
                                 scene_entry_callbacks);
    if (!state.scene_buffers_prepared) {
        return fail(state, callbacks,
                    FieldSessionBootstrapResult::source_data_missing);
    }

    if (input.restored_field_scene.has_value()) {
        const auto& restored = *input.restored_field_scene;
        // 0x594E0 writes FFDDC4 after its 0x03074 call. Do not draw a
        // different area's resources merely because the save names another
        // scene; that resource set has to be supplied by the caller first.
        if (restored.scene_reference >= kFieldSceneProfileCount ||
            restored.scene_reference != input.field_scene.scene_reference) {
            return fail(state, callbacks,
                        FieldSessionBootstrapResult::source_data_missing);
        }

        // The saved camera and live runner positions are followed by
        // 0x028C2, which aligns the camera to the source 0x80 boundary. They
        // do not pass through the pending-position consume in 0x03084.
        state.field_scene_buffers.scene_reference =
            restored.scene_reference;
        state.field_scene_buffers.camera_x = restored.camera_x;
        state.field_scene_buffers.camera_y = restored.camera_y;
        state.field_scene_buffers.source_metadata.initial_field_records =
            restored.runner_positions;
        state.field_scene_entry.camera_x = restored.camera_x;
        state.field_scene_entry.camera_y = restored.camera_y;
        reinitialize_field_scene_entry(state.field_scene_entry,
                                       scene_entry_callbacks);
        state.field_scene_buffers.camera_x = state.field_scene_entry.camera_x;
        state.field_scene_buffers.camera_y = state.field_scene_entry.camera_y;
    }
    state.field_scene_entry_complete = true;

    set_phase(state, callbacks, FieldSessionBootstrapPhase::warmup);
    // ROM 0x008C0: CLR FFDFE8, FFE18A=#1, CLR FFDD22/26, FFDD04/06=#-1.
    state.display_service_word = 0U;
    state.field_lock = 1U;
    state.controller_word = 0U;
    state.controller_aux = 0U;
    state.camera_transition_x = -1;
    state.camera_transition_y = -1;
    // ROM 0x008C0 loads D7 with 0x1E, saves it across the 0x0A8EA call, and
    // DBF executes this body 31 times. The entity service has no semantic
    // counter argument; the host loop keeps the source iteration count local.
    for (std::uint16_t remaining_counter = 0x1EU;; --remaining_counter) {
        if (service_entity_behavior_frames(
                state.behavior_entities, callbacks.entity_behavior_frames) !=
            EntityBehaviorFrameServiceResult::completed) {
            return fail(state, callbacks,
                        FieldSessionBootstrapResult::callback_unresolved);
        }
        callbacks.update_field_camera();
        update_entity_render_pass(state.render_entities,
                                  callbacks.entity_render_pass);
        state.display_dirty_flags = static_cast<std::uint8_t>(
            state.display_dirty_flags | 0x0EU);
        ++state.warmup_iterations;
        if (remaining_counter == 0U) {
            break;
        }
    }

    callbacks.wait_for_vblank();
    state.display_service_word = 0xFFFFU;
    callbacks.advance_title_palette_fade();
    state.field_lock = 0U;
    state.controller_marker = 0U;
    callbacks.sample_controller_input();
    callbacks.sample_controller_input();
    state.display_service_aux = 0U;
    state.display_service_word = 0U;
    state.display_service_marker = 0U;
    callbacks.wait_for_vblank();
    state.controller_samples = 2U;
    state.warmup_complete = true;
    set_phase(state, callbacks, FieldSessionBootstrapPhase::field_ready);
    return FieldSessionBootstrapResult::field_ready;
}

} // namespace shadowrun::ghidra_conversion::integration
