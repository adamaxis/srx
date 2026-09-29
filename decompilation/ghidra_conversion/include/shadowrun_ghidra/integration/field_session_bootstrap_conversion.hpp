#pragma once

#include "shadowrun_ghidra/audio/audio_command_batch_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_action_profile_conversion.hpp"
#include "shadowrun_ghidra/entities/entity_behavior_frame_service_conversion.hpp"
#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"
#include "shadowrun_ghidra/field/field_resource_sequence_conversion.hpp"
#include "shadowrun_ghidra/field/field_scene_entry_conversion.hpp"
#include "shadowrun_ghidra/rendering/display_service_batch_conversion.hpp"
#include "shadowrun_ghidra/world/world_effect_batch_conversion.hpp"
#include "shadowrun_ghidra/world/world_resource_batch_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace shadowrun::ghidra_conversion::integration {

enum class FieldSessionBootstrapPhase {
    not_started,
    mode_entry_audio,
    field_scene_entry,
    warmup,
    field_ready,
    failed,
};

enum class FieldSessionBootstrapResult {
    source_data_missing,
    callback_unresolved,
    field_ready,
};

const char* field_session_bootstrap_phase_name(
    FieldSessionBootstrapPhase phase);

struct FieldSessionBootstrapInput {
    // ROM 0x566A2 / source table 0x166AD8: five 0x22-byte action-profile
    // rows used by field actions and entity behavior.
    std::vector<std::uint8_t> action_selection_profile_source;
    // ROM 0x26938 / source table 0x14C1F4: one 23-word field/mode selection
    // row copied into the working state before field entry.
    std::vector<std::uint16_t> field_mode_selection_state_source;
    // Source-derived scene profiles and the two decoded terrain layers used
    // by the 0x03074 -> 0x0313A field-scene preparation path.
    FieldSceneBufferInput field_scene;
};

struct FieldSessionBootstrapCallbacks {
    // ROM 0x53126: six fixed 0x14 audio transactions, three vblank waits per
    // transaction, then flush plus the zero-command service tick.
    AudioModeEntryCallbacks mode_entry_audio;

    // ROM 0x042F2 entity-slot order initialization, kept as a typed source
    // boundary until its remaining record consumers are fully connected.
    std::function<void()> initialize_entity_order;

    // ROM 0x03074's 0xC7F8 profile/resource preparation boundary.
    std::function<void()> prepare_field_scene_runtime;
    std::function<void()> initialize_scene_profile;
    FieldResourcePreparationCallbacks field_resources;
    std::function<void()> run_world_frame_services;
    std::function<void()> apply_scene_profile_selection;
    FieldViewportPreparationCallbacks field_viewports;
    std::function<void()> upload_scene_profile_block;
    std::function<void()> guard_world_effect_player_spawn;

    // 0x008EE/0x008F6/0x008FC: the three recurring field warm-up services.
    // 0x0A8EA is composed directly; its downstream action/movement/effect
    // calls remain named callbacks inside the typed entity service contract.
    EntityBehaviorFrameCallbacks entity_behavior_frames;
    std::function<void()> update_field_camera;
    EntityRenderPassCallbacks entity_render_pass;

    // 0x00920..0x0093A: end-of-warm-up timing and the two controller samples.
    std::function<void()> wait_for_vblank;
    // The ROM routine is named advance_title_palette_fade in the export; the
    // call is retained here as a source-provenance name even in field mode.
    std::function<void()> advance_title_palette_fade;
    std::function<void()> sample_controller_input;

    std::function<void(FieldSessionBootstrapPhase)> phase_changed;
};

enum class FieldSessionMode : std::uint8_t {
    field = 2U,
};

struct FieldSessionBootstrapState {
    EntityActionProfileTable action_selection_profiles;
    std::vector<std::uint16_t> field_mode_selection_state;
    EntityRuntimeState entity_runtime;
    WorldEffectGlobals world_effect_globals;
    FieldSceneBuffers field_scene_buffers;
    FieldSceneEntryState field_scene_entry;
    std::array<EntityBehaviorFrameRecord, 16> behavior_entities{};
    std::array<RenderEntityState, 16> render_entities{};

    std::vector<std::int32_t> audio_menu_first;
    std::vector<std::int32_t> audio_menu_second;
    FieldSessionMode mode = FieldSessionMode::field;
    std::uint8_t field_resource_mode = 0;
    std::uint8_t display_dirty_flags = 0;
    std::uint8_t controller_marker = 0;
    std::uint8_t display_service_marker = 0;
    std::uint8_t field_lock = 0; // FFE18A
    std::int16_t camera_transition_x = 0; // FFDD04
    std::int16_t camera_transition_y = 0; // FFDD06
    std::uint16_t controller_word = 0; // FFDD22
    std::uint16_t controller_aux = 0; // FFDD26
    std::uint16_t display_service_word = 0; // FFDFE8
    std::uint16_t display_service_aux = 0; // FFDFE6
    std::uint32_t warmup_iterations = 0;
    std::uint32_t controller_samples = 0;
    bool scene_buffers_prepared = false;
    bool audio_mode_entry_complete = false;
    bool field_scene_entry_complete = false;
    bool warmup_complete = false;
    FieldSessionBootstrapPhase phase = FieldSessionBootstrapPhase::not_started;
    std::vector<FieldSessionBootstrapPhase> phase_history;
};

// ROM 0x007F0..0x0094C: compose the source-derived field-mode entry and its
// 31-iteration warm-up using typed host state. This is conversion-runtime
// orchestration only; it does not emulate a processor, VDP, or ROM address
// space, and it is not linked to the native-port runtime.
FieldSessionBootstrapResult bootstrap_field_session(
    const FieldSessionBootstrapInput& input,
    FieldSessionBootstrapState& state,
    const FieldSessionBootstrapCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion::integration
