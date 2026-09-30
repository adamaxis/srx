#pragma once

#include "shadowrun_ghidra/rendering/sprite_record_builder_conversion.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct TitleIntroDisplayClearRequest {
    std::uint16_t transfer_command = 0x3000U;
    std::uint32_t packed_fill_word = 0x07FF07FFU;
    std::uint16_t long_write_count = 0x0400U;
};

// ROM 0x5B29A: clear the title-stage display buffer as one typed host plan.
void clear_title_intro_display_buffer(
    const std::function<void(const TitleIntroDisplayClearRequest&)>& clear);

struct TitleIntroFillRowRequest {
    std::uint16_t destination_offset = 0;
    std::uint16_t word_count = 0;
    std::uint16_t fill_word = 0;
    std::uint16_t row_index = 0;
};

struct TitleIntroWordFillPlan {
    std::uint16_t destination_offset = 0;
    std::uint16_t word_count = 0;
    std::uint16_t row_count = 0;
    std::uint16_t row_stride = 0x0080U;
    std::uint16_t fill_word = 0x07FFU;
};

// ROM 0x5B32C: emit each repeated fill row with its source destination step.
void fill_title_intro_word_block(
    const TitleIntroWordFillPlan& plan,
    const std::function<void(const TitleIntroFillRowRequest&)>& write_row);

constexpr std::size_t kWorldFrameServiceWordCount = 34U;

struct WorldFrameServiceState {
    std::array<std::uint16_t, kWorldFrameServiceWordCount> words{};
    // ROM globals FFDD08/FFDD0C and FFEF14. They are typed host state rather
    // than addresses because the phase dispatcher changes them as part of the
    // scene transition choreography.
    std::int16_t scene_origin_x = 0;
    std::int16_t scene_origin_y = 0;
    std::uint8_t dirty_flags = 0;
};

// ROM 0x5D260: clear 17 longs at FFF11E, then +0=0x11C0, +4=-0x180,
// +0x22=-1, +0x24=0xB4.
void reset_world_frame_service_state(WorldFrameServiceState& state);

enum class WorldSceneEffectPhaseAction {
    no_change,
    play_special_effect_audio,
    load_transition_resource_metadata,
    shift_scene_origin_backward,
    play_scene_transition_audio,
    load_default_resource_metadata,
};

enum class WorldSceneEffectAudioCue : std::uint16_t {
    special_effect = 0x002CU,
    scene_transition = 0x0039U,
};

enum class WorldSceneEffectMetadataSource : std::uint8_t {
    default_mode_resource,
    transition_blank_resource,
};

enum class WorldSceneSpecialEffectBehavior {
    audio_jitter_and_bounce,
};

enum class WorldSceneSpecialEffectSpawnAction {
    spawned,
    no_entity_slot,
    effect_table_full,
};

// ROM 0x7361A fields that survive the 0x5D6BC template copy and are consumed
// by the entity animation/terrain path. The remaining template bytes have no
// proven consumer in this call chain and are not assigned invented meanings.
struct WorldSceneSpecialEffectTemplateFields {
    std::uint16_t animation_accumulator_step = 0x1000U; // +0x18
    std::uint16_t animation_accumulator_limit = 0x1000U; // +0x1C
    std::uint16_t movement_scale = 0x2000U; // +0x2C
    std::int16_t terrain_probe_offset_x = 0x0A02; // +0x4C
    std::int16_t terrain_probe_offset_y = 0x1E00; // +0x4E
    std::uint8_t source_entity_variant = 0x03U; // +0x57
};

struct WorldSceneSpecialEffectRecord {
    WorldSceneSpecialEffectTemplateFields template_fields{};
    std::uint8_t entity_slot = 0; // +0x3E, source slot 9..14
    std::uint16_t world_x = 0x1200U; // +0x00
    std::uint16_t world_y = 0x02C0U; // +0x04
    std::uint16_t behavior_type = 0x0002U; // +0x0E
    bool randomized_motion_enabled = true; // +0x10 bit 7
    bool active = false; // +0x10 bit 0
    std::uint16_t animation_resource = 0x000CU; // +0x16
    std::uint16_t effect_slot = 0; // +0x30
    std::uint16_t effect_record_parameter = 0; // +0x20
    std::uint16_t effect_auxiliary_offset = 0; // +0x26
    std::uint8_t jitter_interval = 0x03U; // +0x41
    std::uint16_t jitter_counter = 0; // +0x44
    std::int16_t lifetime = -1; // +0x46
    WorldSceneSpecialEffectBehavior behavior =
        WorldSceneSpecialEffectBehavior::audio_jitter_and_bounce; // +0x3A
    std::uint16_t effect_profile = 0x002AU; // +0x38
};

struct WorldSceneSpecialEffectSpawnState {
    // The source scans the six world-scene entity records at slots 9..14 and
    // tests only bit 0 of each record's +0x10 flags byte.
    std::array<std::uint8_t, 6> candidate_entity_flags{};
    // ROM 0x1A2CC scans the shared eighteen-entry effect table.
    std::array<std::uint8_t, 18> effect_slot_flags{};
};

struct WorldSceneSpecialEffectSpawnResult {
    WorldSceneSpecialEffectSpawnAction action =
        WorldSceneSpecialEffectSpawnAction::no_entity_slot;
    std::optional<WorldSceneSpecialEffectRecord> record;
};

// ROM 0x5D6BC: claim one of the six pre-title world-scene entity slots,
// install the fixed-position special effect state, then claim an effect slot.
// A full effect table leaves the copied entity record inactive, matching the
// source's final BCLR of +0x10 bit 0.
WorldSceneSpecialEffectSpawnResult spawn_scene_special_effect(
    WorldSceneSpecialEffectSpawnState& state);

struct WorldSceneEffectPhaseCallbacks {
    // ROM 0x530DE: the conversion exposes the source audio command as a
    // named cue; Windows audio playback remains outside this source layer.
    std::function<void(WorldSceneEffectAudioCue)> play_audio;
    // ROM 0x5D6BC: allocate and initialize the fixed-position special effect.
    std::function<WorldSceneSpecialEffectSpawnResult()> spawn_special_effect;
    // ROM 0x566D8: copy one of the two source-selected mode-resource blocks.
    std::function<void(WorldSceneEffectMetadataSource)>
        copy_resource_metadata;
};

struct WorldSceneEffectPhaseResult {
    WorldSceneEffectPhaseAction action =
        WorldSceneEffectPhaseAction::no_change;
    std::uint8_t dirty_flags = 0;
    std::optional<WorldSceneSpecialEffectSpawnResult> special_effect_spawn;
};

// ROM 0x5D416: dispatch the staged pre-title/world-scene effect phase. The
// source's FFDD08/FFDD0C pair is retained as typed scene-origin state here;
// no VDP command or ROM address is part of the host contract.
WorldSceneEffectPhaseResult advance_world_scene_effect_phase(
    WorldFrameServiceState& state,
    const WorldSceneEffectPhaseCallbacks& callbacks);

struct WorldSceneSecondaryEffectState {
    // Source entity record +0x11. Bit 0 is enabled before either child call.
    std::uint8_t behavior_flags = 0;
};

struct WorldSceneSecondaryEffectCallbacks {
    // ROM 0x15930: apply the selected entity's secondary effect.
    std::function<void()> apply_entity_secondary_effect;
    // ROM 0x0AFF2: prepare the fixed world-scene auxiliary behavior record.
    std::function<void()> prepare_auxiliary_behavior;
};

// ROM 0x5D3E6: mark the selected world-scene entity, then apply its secondary
// effect and prepare the auxiliary behavior in that exact order.
void spawn_world_scene_secondary_effect(
    WorldSceneSecondaryEffectState& state,
    const WorldSceneSecondaryEffectCallbacks& callbacks);

constexpr std::array<std::array<std::int16_t, 2>, 8> kWorldSceneEffectHeadingOffsets{{
    {{2, 41}}, {{5, 41}}, {{4, 41}}, {{7, 41}},
    {{6, 41}}, {{7, 41}}, {{5, 41}}, {{5, 41}},
}};

struct WorldSceneEffectSpriteBatch {
    std::uint16_t frame_index = 0;
    std::uint8_t first_sequence = 0;
    std::uint8_t next_sequence = 0;
    std::vector<BuiltSpriteRecord> records;
};

// ROM 0x1378 consumes each selected sentinel-terminated source list and
// appends every record, not just the first source pointer. The source lists
// are represented as typed host data in the implementation file.

// ROM 0x5D55C: advance the shared eight-frame scene-effect animation counter.
void advance_world_scene_effect_frame(WorldFrameServiceState& state);

// ROM 0x5D5A2: +6 countdown reload 0x14, +0x16 wrap 0..8, then 0x1378.
WorldSceneEffectSpriteBatch advance_world_scene_effect_timer_a(
    WorldFrameServiceState& state,
    std::uint8_t first_sequence = 0);

// ROM 0x5D5FC: +0x1A reload 0xA, +0x1C wrap 0..9, D2-0x18.
WorldSceneEffectSpriteBatch advance_world_scene_effect_timer_b(
    WorldFrameServiceState& state,
    std::uint8_t first_sequence = 0);

// ROM 0x5D65A: +0x20 reload 4, +0x1E wrap 0..0x27, D2+0x30 D1+0x12.
WorldSceneEffectSpriteBatch advance_world_scene_effect_timer_c(
    WorldFrameServiceState& state,
    std::uint8_t first_sequence = 0);

// ROM 0x5D57E: use the shared scene frame to select the refresh list, then
// build all of its records at the projected scene origin without the timer
// leaves' heading offsets.
WorldSceneEffectSpriteBatch refresh_world_scene_effect_sprites(
    const WorldFrameServiceState& state,
    std::uint8_t first_sequence = 0);

struct WorldSceneEffectEntitySlot {
    // Source entity record fields used by the six-slot scan and the
    // secondary-effect handoff: +0x10, +0x11, +0x4C, and +0x9C.
    std::uint8_t record_flags = 0;
    std::uint8_t behavior_flags = 0;
    std::uint8_t secondary_effect_trigger = 0;
    std::uint8_t secondary_effect_phase = 0;
};

struct WorldSceneEntityEffectRuntimeState {
    std::uint8_t scene_mode = 0; // FFE186
    std::uint8_t sprite_sequence = 0; // D5 entering the 0x1378 call chain
    std::array<WorldSceneEffectEntitySlot, 6> candidate_entities{};
    WorldSceneEffectEntitySlot auxiliary_entity{}; // fixed FF0400 record
    bool terminal_auxiliary_marker = false;
};

struct WorldSceneEntityEffectCallbacks {
    // ROM 0x5A56: project the scene entity before any sprite-table timer leaf.
    std::function<void()> project_entity_screen_position;
    // ROM 0x1378: publish every record from each selected source list in
    // source order; the batch also carries the next D5 sequence value.
    std::function<void(const WorldSceneEffectSpriteBatch&)> publish_sprites;
    // ROM 0x5D57E: rebuild the remaining scene-effect sprite records.
    std::function<void(const WorldSceneEffectSpriteBatch&)>
        refresh_world_scene_effect_sprites;
    // ROM 0x5D408: stage-zero phase dispatch.
    WorldSceneEffectPhaseCallbacks phase_callbacks;
    // ROM 0x5BA66: resource-index installation after the initial wait.
    std::function<void()> install_scene_resource_index_entries;
    // ROM 0x1E2E0: terminal world-activity handoff.
    std::function<void()> begin_world_activity_if_idle;
    // The fixed FF0400 record receives three setup bytes before the final
    // secondary-effect call. Their concrete meaning remains a separate record
    // audit, so the write is an explicit typed boundary here.
    std::function<void()> prepare_terminal_auxiliary_entity;
    // Child calls reached through ROM 0x5D3E6, bound to the selected record.
    std::function<void(WorldSceneEffectEntitySlot&)>
        apply_entity_secondary_effect;
    std::function<void(WorldSceneEffectEntitySlot&)>
        prepare_entity_auxiliary_behavior;
};

enum class WorldSceneEntityEffectAction {
    no_change,
    advanced_resource_phase,
    entered_active_resource_stage,
    rendered_active_effect,
    selected_existing_entity_effect,
    spawned_terminal_auxiliary_effect,
    completed_world_activity,
    entered_final_effect_stage,
};

struct WorldSceneEntityEffectResult {
    WorldSceneEntityEffectAction action =
        WorldSceneEntityEffectAction::no_change;
    std::optional<std::size_t> selected_entity_index;
};

// ROM 0x5D29C: coordinate the staged world-scene effect. The caller supplies
// typed presentation/entity/resource callbacks for the leaves at 0x5A56,
// 0x1378, 0x5D57E, 0x5BA66, 0x1E2E0, and 0x5D3E6; no indirect record pointer
// or Genesis address is retained by the host implementation.
WorldSceneEntityEffectResult advance_world_scene_entity_effect(
    WorldFrameServiceState& frame_state,
    WorldSceneEntityEffectRuntimeState& runtime_state,
    const WorldSceneEntityEffectCallbacks& callbacks);

constexpr std::array<SpriteSourceRecord, 2> kTitleIntroSceneEntitySource{{
    {6, -15, 4, 0x04F6U},
    {1, -7, 8, 0x04F8U},
}};

// ROM 0x5A37C: A2=0x1DF1B2, D1=FFDD46, D2=FFDD48, D6=0x2000, jmp 0x1378.
std::vector<BuiltSpriteRecord> render_title_intro_scene_entities(
    std::int16_t y_offset,
    std::int16_t x_offset,
    std::uint8_t first_sequence);

} // namespace shadowrun::ghidra_conversion
