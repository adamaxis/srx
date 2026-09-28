#include "shadowrun_ghidra/integration/field_session_bootstrap_conversion.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

namespace {

using namespace shadowrun::ghidra_conversion;
using namespace shadowrun::ghidra_conversion::integration;

struct Observations {
    std::vector<std::string> order;
    std::vector<std::uint16_t> audio_commands;
    std::vector<std::uint16_t> audio_wait_counters;
    std::uint32_t behavior_calls = 0;
    std::uint32_t camera_calls = 0;
    std::uint32_t render_passes = 0;
    std::uint32_t vblank_calls = 0;
    std::uint32_t palette_calls = 0;
    std::uint32_t controller_samples = 0;
    std::vector<FieldSessionBootstrapPhase> phases;
};

void record(Observations& observations, const char* name) {
    observations.order.emplace_back(name);
}

FieldSessionBootstrapInput make_input() {
    FieldSessionBootstrapInput input;
    input.action_selection_profile_source.resize(
        kEntityActionProfileCount * kEntityActionProfileRowSize, 0U);
    input.action_selection_profile_source[0] = 0x00U;
    input.action_selection_profile_source[1] = 0x05U;
    input.action_selection_profile_source[2] = 0x10U;
    input.action_selection_profile_source[3] = 0x20U;
    input.field_mode_selection_state_source.resize(
        kFieldModeSelectionStateWordCount, 0U);
    input.field_mode_selection_state_source.front() = 0x1234U;
    input.field_mode_selection_state_source.back() = 0x5678U;
    input.field_scene.normal_profile[0] = 0x0100U;
    input.field_scene.normal_profile[1] = 0x0200U;
    input.field_scene.alternate_profile[0] = 0x0300U;
    input.field_scene.alternate_profile[1] = 0x0400U;
    input.field_scene.scene_reference = 6U;
    input.field_scene.terrain_stream = {0xCA, 0xFE, 0xBA, 0xBE};
    input.field_scene.layer_a.resize(kFieldTerrainLayerWordCount, 0x11U);
    input.field_scene.layer_b.resize(kFieldTerrainLayerWordCount, 0x22U);
    input.field_scene.layer_a.front() = 0xAAAAU;
    input.field_scene.layer_b.back() = 0xBBBBU;
    input.field_scene.source_metadata.available = true;
    input.field_scene.source_metadata.initial_camera_x_fixed = 0x0A00U;
    input.field_scene.source_metadata.initial_camera_y_fixed = 0x0280U;
    input.field_scene.source_metadata.initial_field_records[0] =
        {0x1310U, 0x0980U};
    input.field_scene.source_metadata.event_mode_table[0] = 0x15U;
    input.field_scene.source_metadata.terrain_collision_rectangles.push_back(
        {0x0000U, 0x005FU, 0x0000U, 0x007FU, 0x7FU});
    input.field_scene.source_metadata.terrain_collision_map[0U] = 0x7FU;
    return input;
}

FieldSessionBootstrapCallbacks make_callbacks(Observations& observations) {
    FieldSessionBootstrapCallbacks callbacks{};
    callbacks.mode_entry_audio.run_fixed_audio_transaction =
        [&observations](const std::uint32_t command) {
            observations.audio_commands.push_back(
                static_cast<std::uint16_t>(command));
        };
    callbacks.mode_entry_audio.wait_for_vblank_frames =
        [&observations](const std::uint16_t counter) {
            observations.audio_wait_counters.push_back(counter);
        };
    callbacks.mode_entry_audio.flush_audio_bus = [&observations] {
        record(observations, "audio.flush");
    };
    callbacks.mode_entry_audio.service_audio_tick = [&observations] {
        record(observations, "audio.tick");
    };

    callbacks.initialize_entity_order = [&observations] {
        record(observations, "entity.order");
    };
    callbacks.initialize_scene_profile = [&observations] {
        record(observations, "scene.profile");
    };
    callbacks.field_resources.prepare_asset_setup = [&observations] {
        record(observations, "resources.setup");
    };
    callbacks.field_resources.upload_asset_sequence = [&observations] {
        record(observations, "resources.upload");
    };
    callbacks.field_resources.refresh_animation_assets = [&observations] {
        record(observations, "resources.animation");
    };
    callbacks.field_resources.load_font = [&observations] {
        record(observations, "resources.font");
    };
    callbacks.run_world_frame_services = [&observations] {
        record(observations, "world.services");
    };
    callbacks.apply_scene_profile_selection = [&observations] {
        record(observations, "scene.select");
    };
    callbacks.field_viewports.prepare_scene_resources = [&observations] {
        record(observations, "viewport.resources");
    };
    callbacks.field_viewports.refresh_mode_state = [&observations] {
        record(observations, "viewport.mode");
    };
    callbacks.field_viewports.special_effects_enabled = [] { return false; };
    callbacks.field_viewports.service_special_effects = [&observations] {
        record(observations, "viewport.effects");
    };
    callbacks.field_viewports.render_normal_viewport = [&observations] {
        record(observations, "viewport.normal");
    };
    callbacks.field_viewports.render_alternate_viewport = [&observations] {
        record(observations, "viewport.alternate");
    };
    callbacks.upload_scene_profile_block = [&observations] {
        record(observations, "scene.upload");
    };
    callbacks.guard_world_effect_player_spawn = [&observations] {
        record(observations, "world.spawn_guard");
    };

    callbacks.entity_behavior_frames.dispatch_entity_action_candidates =
        [&observations] { ++observations.behavior_calls; };
    callbacks.entity_behavior_frames.noop_entity_action_hook =
        [](EntityBehaviorFrameRecord&) { return static_cast<std::uint8_t>(0); };
    callbacks.entity_behavior_frames.complete_entity_behavior_transition =
        [](EntityBehaviorFrameRecord&) {};
    callbacks.entity_behavior_frames.clear_effect_slot_state =
        [](EntityBehaviorFrameRecord&) {};
    callbacks.entity_behavior_frames.handle_mode_action_event =
        [](EntityBehaviorFrameRecord&) {};
    callbacks.entity_behavior_frames.advance_entity_behavior_action =
        [](EntityBehaviorFrameRecord&) {};
    callbacks.entity_behavior_frames.enter_entity_behavior_transition =
        [](EntityBehaviorFrameRecord&) {};
    callbacks.entity_behavior_frames.dispatch_entity_collision_behavior =
        [](EntityBehaviorFrameRecord&) {};
    callbacks.entity_behavior_frames.apply_mode_five_boundary_motion =
        [](EntityBehaviorFrameRecord&) {};
    callbacks.entity_behavior_frames.resolve_entity_proximity_interactions =
        [](EntityBehaviorFrameRecord&) {};
    callbacks.entity_behavior_frames.clear_entity_effect_if_forced =
        [](EntityBehaviorFrameRecord&) {};
    callbacks.entity_behavior_frames.resolve_entity_fallback_movement =
        [](EntityBehaviorFrameRecord&) {};
    callbacks.entity_behavior_frames.run_entity_continuation =
        [](EntityBehaviorFrameRecord&) {};
    callbacks.update_field_camera =
        [&observations] { ++observations.camera_calls; };
    callbacks.entity_render_pass.project_screen_position =
        [](RenderEntityState&) {};
    callbacks.entity_render_pass.advance_animation =
        [](RenderEntityState&) {};
    callbacks.entity_render_pass.finish_entity = [](RenderEntityState&) {};
    callbacks.entity_render_pass.finish_pass =
        [&observations] { ++observations.render_passes; };
    callbacks.wait_for_vblank =
        [&observations] { ++observations.vblank_calls; };
    callbacks.advance_title_palette_fade =
        [&observations] { ++observations.palette_calls; };
    callbacks.sample_controller_input =
        [&observations] { ++observations.controller_samples; };
    callbacks.phase_changed = [&observations](const FieldSessionBootstrapPhase phase) {
        observations.phases.push_back(phase);
    };
    return callbacks;
}

} // namespace

int main() {
    auto input = make_input();
    Observations observations;
    auto callbacks = make_callbacks(observations);
    FieldSessionBootstrapState state;
    state.audio_menu_first = {1, 2, 3};
    state.audio_menu_second = {4, 5, 6};
    state.render_entities[0].active = true;

    assert(bootstrap_field_session(input, state, callbacks) ==
           FieldSessionBootstrapResult::field_ready);
    assert(state.phase == FieldSessionBootstrapPhase::field_ready);
    assert(state.audio_mode_entry_complete && state.field_scene_entry_complete &&
           state.warmup_complete);
    assert(state.audio_menu_first.size() == 35U &&
           state.audio_menu_second.size() == 35U);
    assert(state.entity_runtime.entity_table_ready);
    assert(state.action_selection_profiles.rows[0].event_count_last_index == 5U);
    assert(state.field_mode_selection_state.size() ==
           kFieldModeSelectionStateWordCount);
    assert(state.field_scene_buffers.scene_mode == 1U &&
           state.field_scene_buffers.camera_x == 0x0300 &&
           state.field_scene_buffers.camera_y == 0x0400 &&
           state.field_scene_buffers.source_metadata.available &&
           state.field_scene_buffers.source_metadata.initial_camera_x_fixed ==
               0x0A00U &&
           state.field_scene_buffers.source_metadata.initial_field_records[0].x ==
               0x1310U &&
           state.field_scene_buffers.source_metadata.event_mode_table[0] ==
               0x15U &&
           state.field_scene_buffers.source_metadata.terrain_collision_map[0U] ==
               0x7FU &&
           state.field_scene_buffers.layer_a.size() ==
               kFieldTerrainLayerWordCount);
    assert(state.warmup_iterations == 31U && observations.behavior_calls == 31U &&
           observations.camera_calls == 31U && observations.render_passes == 31U);
    assert(state.display_dirty_flags == 0x0EU && observations.vblank_calls == 1U &&
           observations.palette_calls == 1U &&
           observations.controller_samples == 2U);
    assert(observations.audio_commands.size() == 6U &&
           std::all_of(observations.audio_commands.begin(),
                       observations.audio_commands.end(),
                       [](const std::uint16_t command) { return command == 0x14U; }));
    assert(observations.audio_wait_counters.size() == 6U &&
           std::all_of(observations.audio_wait_counters.begin(),
                       observations.audio_wait_counters.end(),
                       [](const std::uint16_t counter) { return counter == 2U; }));
    assert((observations.phases ==
            std::vector<FieldSessionBootstrapPhase>{
                FieldSessionBootstrapPhase::mode_entry_audio,
                FieldSessionBootstrapPhase::field_scene_entry,
                FieldSessionBootstrapPhase::warmup,
                FieldSessionBootstrapPhase::field_ready}));

    auto incomplete_input = input;
    incomplete_input.field_mode_selection_state_source.resize(2U);
    FieldSessionBootstrapState incomplete_state;
    assert(bootstrap_field_session(incomplete_input, incomplete_state,
                                   callbacks) ==
           FieldSessionBootstrapResult::source_data_missing);
    assert(incomplete_state.phase == FieldSessionBootstrapPhase::failed);

    auto missing_callback = callbacks;
    missing_callback.update_field_camera = {};
    FieldSessionBootstrapState unresolved_state;
    assert(bootstrap_field_session(input, unresolved_state, missing_callback) ==
           FieldSessionBootstrapResult::callback_unresolved);
    assert(unresolved_state.phase == FieldSessionBootstrapPhase::failed);
    return 0;
}
