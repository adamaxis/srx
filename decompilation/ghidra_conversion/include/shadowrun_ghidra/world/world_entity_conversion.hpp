#pragma once

#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"
#include "shadowrun_ghidra/rendering/sprite_record_builder_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

namespace shadowrun::ghidra_conversion {

struct WorldFrameState {
    bool audio_pending = false;
    bool world_dirty = false;
    bool effect_record_active = false;
    bool effect_transition_pending = false;
    bool terrain_copy_pending = false;
    bool terrain_mode_active = false;
    bool compressed_entity_data_pending = false;
    std::uint8_t scene_state = 0;
};

struct WorldFrameCallbacks {
    std::function<void()> service_audio;
    std::function<void()> reset_effect_record;
    std::function<void()> begin_effect_transition;
    std::function<void()> clear_effect_slots;
    std::function<void()> clear_entity_effects;
    std::function<void()> clear_scene_pending;
    std::function<void()> finalize_entity_templates;
    std::function<void()> service_scene_effect;
    std::function<void()> copy_terrain_layers;
    std::function<void()> process_terrain_entries;
    std::function<void()> rebuild_collision_map;
    std::function<void()> load_scene_resources;
    std::function<void()> clear_scene_transitions;
    std::function<void()> populate_entities;
    std::function<void()> dispatch_entity_actions;
    std::function<void()> decode_entity_data;
    std::function<void()> mark_entities_for_render;
};

// ROM 0x02900: one world/entity frame-service pass in source order.
void run_world_frame_services(WorldFrameState& state,
                              const WorldFrameCallbacks& callbacks);

struct EntityFrameRecord {
    bool active = false;
    std::uint8_t behavior = 0;
    std::uint8_t flags = 0;
    std::uint8_t mode = 0;
    std::int16_t timer = 0;
    std::int16_t coordinate = 0;
    bool special_descriptor = false;
};

struct EntityFrameCallbacks {
    std::function<void(std::array<EntityFrameRecord, 16>&)> sort_by_depth;
    std::function<void(EntityFrameRecord&)> behavior_six;
    std::function<void(EntityFrameRecord&)> behavior_thirteen;
    std::function<void(EntityFrameRecord&)> behavior_table;
    std::function<void(EntityFrameRecord&)> secondary_timer;
    std::function<void(EntityFrameRecord&)> primary_timer;
    std::function<void(EntityFrameRecord&)> delayed_effect;
    std::function<void(EntityFrameRecord&)> animated_effect;
    std::function<void(EntityFrameRecord&)> sprite_burst;
    std::function<void(EntityFrameRecord&)> expand_sprites;
    std::function<void()> finish_effect;
    std::function<void()> finish_wave;
    std::function<void()> finish_post_state;
};

// ROM 0x04658: update the sixteen active entity slots and finish the shared
// effect/object passes without carrying source record pointers into runtime.
void update_entity_frame(std::array<EntityFrameRecord, 16>& entities,
                         const EntityFrameCallbacks& callbacks);

void refresh_projected_entity_sprites(
    const std::array<EntityFrameRecord, 16>& entities,
    const std::function<void(const EntityFrameRecord&)>& project_entity,
    const std::function<void(const EntityFrameRecord&)>& build_sprite);

struct ProjectedEntitySpriteSlot {
    std::uint16_t world_x = 0;
    std::int16_t type = 0;
    std::uint16_t world_y = 0;
    std::uint16_t sprite_offset_y = 0;
    std::uint16_t screen_x = 0;
    std::uint16_t screen_y = 0;
    std::uint8_t flags = 0;
};

constexpr std::array<std::uint32_t, 22> kProjectedEntitySpriteSources{
    0x00005F78U, 0x00005F78U, 0x00005F78U, 0x00005F78U, 0x00005F78U,
    0x00005F78U, 0x00005F78U, 0x00005F78U, 0x00005F78U, 0x00005F78U,
    0x00005F78U, 0x00005F78U, 0x000095A4U, 0x000095A4U, 0x000095A4U,
    0x00005F96U, 0x00005F96U, 0x00005F96U, 0x00005DEAU, 0x00005DEAU,
    0x00005F8CU, 0x00005FA0U,
};

// ROM 0x481E type word +2: table 0x48B2 when 0..0x62, 0x5FAA at 0x64,
// 0x1D47C otherwise; negatives use 0x5F82 when high byte is 0xC0.
std::uint32_t select_projected_entity_sprite_source(std::int16_t type);

// ROM 0x481E: ten FF164C slots, bit 0 of +0x10, project 0x5A62, D6=0x4000.
std::vector<BuiltSpriteRecord> refresh_projected_entity_sprites(
    std::array<ProjectedEntitySpriteSlot, 10>& slots,
    std::uint16_t camera_x,
    std::uint16_t camera_y,
    std::uint8_t first_sequence);

struct EntityEffectSlotState {
    bool active = false;
    std::uint16_t tile_index = 0;
    std::uint16_t source_entity = 0;
    std::uint8_t variant = 0;
    std::int16_t countdown = 0;
    std::uint8_t phase = 0;
    std::array<std::uint8_t, 4> restored_tiles{};
};

struct EntityEffectSource {
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t heading = 0;
    std::uint8_t item_code = 0;
};

void spawn_entity_effect(
    EntityEffectSlotState& effect,
    const EntityEffectSource& source,
    std::uint16_t slot_index,
    const std::function<std::uint16_t(std::uint16_t, std::uint16_t)>&
        project_tile,
    const std::function<void(const EntityEffectSlotState&)>& append_sprite,
    const std::function<void()>& service_audio);

void advance_entity_effect_timer(
    EntityEffectSlotState& effect,
    const std::function<void()>& clear_slot,
    const std::function<void()>& restore_tiles,
    const std::function<void()>& project_entity,
    const std::function<void(std::uint8_t)>& build_sprite_records,
    const std::function<void(const EntityEffectSlotState&)>& append_sprite);

struct EntityEffectTimerState {
    std::int16_t countdown = 0;
    std::uint16_t heading = 0;
    std::uint16_t world_x = 0;
    std::uint16_t world_y = 0;
    std::uint16_t sprite_offset_y = 0;
    std::uint16_t screen_x = 0;
    std::uint16_t screen_y = 0;
    std::int16_t attribute_offset = 0;
    std::uint16_t slot = 0;
    std::uint16_t collision_base = 0;
    std::array<std::uint8_t, 5> saved_tiles{};
};

constexpr std::array<std::array<std::pair<std::int16_t, std::int16_t>, 5>, 8>
    kEffectTimerHeadingOffsets{{
        {{{0, 0}, {32, 0}, {-32, 0}, {64, 0}, {-64, 0}}},
        {{{0, 0}, {-32, 0}, {-64, 0}, {0, 32}, {0, 64}}},
        {{{0, 0}, {0, -32}, {0, 32}, {0, -64}, {0, 64}}},
        {{{0, 0}, {-32, 0}, {-64, 0}, {0, -32}, {0, -64}}},
        {{{0, 0}, {32, 0}, {-32, 0}, {64, 0}, {-64, 0}}},
        {{{0, 0}, {32, 0}, {64, 0}, {0, -32}, {0, -64}}},
        {{{0, 0}, {0, -32}, {0, 32}, {0, -64}, {0, 64}}},
        {{{0, 0}, {32, 0}, {64, 0}, {0, 32}, {0, 64}}},
    }};

constexpr std::array<std::uint16_t, 4> kEffectTimerSpriteAttributes{
    0x0000U, 0x1800U, 0x1000U, 0x0800U};

// ROM 0x4ADC: sub +0xE, expire to 0x4B6C, else two of five 0x13BE attaches.
std::vector<BuiltSpriteRecord> advance_entity_effect_timer(
    EntityEffectTimerState& state,
    std::array<std::uint8_t, 18>& slots,
    std::vector<std::uint8_t>& collision_map,
    std::uint16_t camera_x,
    std::uint16_t camera_y,
    std::uint8_t first_sequence);

struct EffectAnimationState {
    std::int16_t countdown = 0;
    std::uint8_t phase = 0;
    std::uint8_t phase_limit = 3;
    std::int16_t lifetime = 0;
    bool active = true;
};

bool advance_effect_animation(
    EffectAnimationState& state,
    const std::function<void()>& clear_slot,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprite_records);

struct EffectAnimationSpriteState {
    EffectAnimationState animation{};
    std::uint16_t facing = 0;
    std::uint16_t screen_x = 0;
    std::uint16_t screen_y = 0;
    std::int16_t attribute_offset = 0;
    std::uint16_t slot = 0;
    std::uint8_t entity_flags = 0;
};

constexpr std::array<std::pair<std::int16_t, std::int16_t>, 8>
    kEffectAnimationHeadingOrigins{{
        {-1, -28}, {21, -23}, {24, -5}, {18, 6},
        {2, 13}, {-17, 6}, {-23, -5}, {-17, -21},
    }};

constexpr std::array<std::array<SpriteSourceRecord, 4>, 8>
    kEffectAnimationSprites{{
        {SpriteSourceRecord{-6, -11, 5, 0}, SpriteSourceRecord{-5, -24, 7, 0},
         SpriteSourceRecord{-4, -21, 6, 0}, SpriteSourceRecord{-3, -16, 1, 0}},
        {SpriteSourceRecord{-7, -8, 5, 0}, SpriteSourceRecord{-1, -16, 10, 0},
         SpriteSourceRecord{0, -15, 9, 0}, SpriteSourceRecord{-3, -17, 10, 0}},
        {SpriteSourceRecord{1, -7, 5, 0}, SpriteSourceRecord{0, -6, 13, 0},
         SpriteSourceRecord{1, -4, 8, 0}, SpriteSourceRecord{-4, -7, 9, 0}},
        {SpriteSourceRecord{-7, -8, 5, 0x1000U},
         SpriteSourceRecord{-1, -8, 10, 0x1000U},
         SpriteSourceRecord{0, -1, 9, 0x1000U},
         SpriteSourceRecord{-3, -7, 10, 0x1000U}},
        {SpriteSourceRecord{-6, -5, 5, 0x1000U},
         SpriteSourceRecord{-5, -8, 7, 0x1000U},
         SpriteSourceRecord{-4, -3, 6, 0x1000U},
         SpriteSourceRecord{-3, 0, 1, 0x1000U}},
        {SpriteSourceRecord{-9, -8, 5, 0x1800U},
         SpriteSourceRecord{-23, -8, 10, 0x1800U},
         SpriteSourceRecord{-24, -1, 9, 0x1800U},
         SpriteSourceRecord{-21, -7, 10, 0x1800U}},
        {SpriteSourceRecord{-17, -7, 5, 0x0800U},
         SpriteSourceRecord{-32, -6, 13, 0x0800U},
         SpriteSourceRecord{-25, -4, 8, 0x0800U},
         SpriteSourceRecord{-20, -7, 9, 0x0800U}},
        {SpriteSourceRecord{-9, -8, 5, 0x0800U},
         SpriteSourceRecord{-23, -16, 10, 0x0800U},
         SpriteSourceRecord{-24, -15, 9, 0x0800U},
         SpriteSourceRecord{-21, -17, 10, 0x0800U}},
    }};

// ROM 0x4E86: +2 cadence 6, phase +3 expire at 4, 0x4F42/0x4F22, jmp 0x13BE.
std::vector<BuiltSpriteRecord> advance_effect_animation(
    EffectAnimationSpriteState& state,
    std::array<std::uint8_t, 18>& slots,
    std::uint8_t first_sequence);
bool advance_effect_animation_variant(
    EffectAnimationState& state,
    const std::function<void()>& clear_slot,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprite_records);

struct EffectAnimationVariantState {
    EffectAnimationState animation{};
    std::uint16_t screen_x = 0;
    std::uint16_t screen_y = 0;
    std::int16_t attribute_offset = 0;
    std::uint8_t auxiliary = 0;
    std::uint16_t jitter_bit = 0;
};

constexpr std::array<SpriteSourceRecord, 5> kEffectAnimationVariantSprites{{
    {-14, -27, 15, 0},
    {-14, -27, 15, 0},
    {-11, -24, 10, 0},
    {-14, -27, 15, 0},
    {-13, -27, 15, 0},
}};

// ROM 0x5662: +2 cadence 6, phase expire at 5 clears +0xB2, 0x584A, D6 0x6000.
std::vector<BuiltSpriteRecord> advance_effect_animation_variant(
    EffectAnimationVariantState& state,
    std::uint8_t first_sequence);

void run_effect_variant_a(
    EntityEffectSlotState& effect,
    const std::function<void()>& clone_variant,
    const std::function<void()>& prepare_auxiliary,
    const std::function<void()>& clear_slot);

enum class EffectVariantScanResult {
    gated,
    jitter_live,
    child_cloned,
    auxiliary_cloned,
    expired,
};

struct EffectVariantRecord {
    EntitySpawnCloneRecord spawn{};
    std::uint16_t screen_x = 0;
    std::uint16_t screen_y = 0;
    std::uint8_t flags = 0;
    std::int16_t kind = 0;
    std::uint8_t discriminator = 0;
    std::uint8_t generation = 0;
};

struct EffectVariantSharedState {
    std::int16_t gate = 0;
    std::int16_t delay = 0;
    std::int16_t phase_gate = 0;
    std::int16_t offset_x = 0;
    std::int16_t offset_y = 0;
    std::uint16_t random_word = 0;
    std::uint16_t seed = 0;
    std::array<std::uint8_t, 18> slots{};
    EntityRuntimeCleanupState cleanup{};
};

struct EffectVariantScanOutput {
    EffectVariantScanResult result = EffectVariantScanResult::gated;
    std::vector<BuiltSpriteRecord> sprites;
    std::size_t cloned_index = 0;
};

// ROM 0x53C4: FFDFF6/FFDFF8 gate, 0x5494, nine +0x100 scan, 0x5468/0x1A304/0x5622.
EffectVariantScanOutput run_effect_variant_a(
    EffectVariantRecord& self,
    std::array<EffectVariantRecord, 9>& neighbors,
    EffectVariantRecord& auxiliary,
    EffectVariantSharedState& shared,
    std::uint8_t first_sequence);
void run_effect_variant_b(
    EntityEffectSlotState& effect,
    const std::function<void()>& clone_variant,
    const std::function<void()>& prepare_auxiliary,
    const std::function<void()>& clear_slot);

// ROM 0x557E: FFDFF2/FFDFF4 gate, 0x5662, nine +0x100 scan, 0x563E/0x1A304/0x5622.
EffectVariantScanOutput run_effect_variant_b(
    EffectVariantRecord& self,
    std::array<EffectVariantRecord, 9>& neighbors,
    EffectVariantRecord& auxiliary,
    EffectVariantSharedState& shared,
    std::uint8_t first_sequence);
void advance_entity_animation_effect(
    EffectAnimationState& state,
    const std::function<void()>& build_special_sprite,
    const std::function<void()>& build_sprite_records);

struct EntityAnimationEffectState {
    std::int8_t countdown = 0;
    std::uint8_t phase = 0;
    std::uint16_t screen_x = 0;
    std::uint16_t screen_y = 0;
    std::uint16_t marker = 0;
};

constexpr std::array<SpriteSourceRecord, 4> kEntityAnimationEffectSprites{{
    {-14, -32, 14, 0x0580U},
    {-14, -32, 14, 0x058CU},
    {-14, -32, 14, 0x0598U},
    {-14, -32, 14, 0x05A4U},
}};

// ROM 0x5936: +6 cadence 3, phase +7 wrap 4, 0x5A46, D6 0x4000; +0xE==0xF dual.
std::vector<BuiltSpriteRecord> advance_entity_animation_effect(
    EntityAnimationEffectState& state,
    std::uint8_t first_sequence);
void refresh_delayed_effect_sprites(
    EntityEffectSlotState& effect,
    const std::function<void()>& build_sprite_records,
    const std::function<void()>& append_sprite);

constexpr std::array<SpriteSourceRecord, 4> kDelayedEffectSprites{{
    {-12, -32, 13, 0x045FU},
    {-12, -29, 12, 0x0467U},
    {-20, -32, 13, 0x0C5FU},
    {-20, -29, 12, 0x0C67U},
}};

// ROM 0x59EA: +6 cadence 3, phase +7 wrap 4, 0x5A36, D2+0xC, D6 0x2000.
std::vector<BuiltSpriteRecord> refresh_delayed_effect_sprites(
    EntityAnimationEffectState& state,
    std::uint8_t first_sequence);

// ROM 0x060F4: set the source carry condition used by the descriptor caller
// to take its terminal/append branch. It has no data payload, but it is not a
// no-op and is represented as an explicit host decision.
bool set_sprite_descriptor_carry();

struct JitterEffectState {
    EffectAnimationState animation;
    std::int16_t remaining = 0;
    std::int16_t offset_x = 0;
    std::int16_t offset_y = 0;
    std::uint32_t random_state = 0;
    std::uint8_t behavior = 1;
};

void advance_jitter_effect(
    JitterEffectState& state,
    const std::function<std::uint32_t(std::uint32_t&)>& next_random,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprite_records);

struct JitterEffectSpriteState {
    EffectAnimationState animation{};
    std::uint16_t screen_x = 0;
    std::uint16_t screen_y = 0;
    std::int16_t attribute_offset = 0;
    std::uint8_t auxiliary = 0;
    std::int16_t remaining = 0;
    std::int16_t offset_x = 0;
    std::int16_t offset_y = 0;
    std::uint16_t random_word = 0;
    std::uint16_t seed = 0;
};

constexpr std::array<std::array<SpriteSourceRecord, 3>, 2> kJitterEffectSprites{{
    {SpriteSourceRecord{-3, -22, 1, 0}, SpriteSourceRecord{-10, -28, 10, 0},
     SpriteSourceRecord{-3, -22, 1, 0}},
    {SpriteSourceRecord{-3, -22, 1, 0}, SpriteSourceRecord{-6, -27, 5, 0},
     SpriteSourceRecord{-3, -22, 1, 0}},
}};

// ROM 0x5494: +2 cadence 6, phase>=3 ticks FFFE004, 0xD44 jitter, D6 0x6000.
std::vector<BuiltSpriteRecord> advance_jitter_effect(
    JitterEffectSpriteState& state,
    std::uint8_t first_sequence);

} // namespace shadowrun::ghidra_conversion
