#include "shadowrun_ghidra/world/world_entity_conversion.hpp"

#include "shadowrun_ghidra/entities/entity_projection_conversion.hpp"
#include "shadowrun_ghidra/field/field_entity_render_batch_conversion.hpp"

namespace shadowrun::ghidra_conversion {

namespace {

void call_once(const std::function<void()>& callback) {
    if (callback) {
        callback();
    }
}

} // namespace

void run_world_frame_services(WorldFrameState& state,
                              const WorldFrameCallbacks& callbacks) {
    if (state.audio_pending) {
        state.audio_pending = false;
        call_once(callbacks.service_audio);
    }
    if (state.world_dirty) {
        if (state.effect_record_active) {
            call_once(callbacks.reset_effect_record);
        }
        state.world_dirty = false;
        if (state.effect_transition_pending) {
            state.effect_transition_pending = false;
            call_once(callbacks.begin_effect_transition);
        }
        call_once(callbacks.clear_effect_slots);
        call_once(callbacks.clear_entity_effects);
        call_once(callbacks.clear_scene_pending);
        call_once(callbacks.finalize_entity_templates);
        if (state.scene_state != 3U) {
            call_once(callbacks.service_scene_effect);
        }
        if (!state.terrain_copy_pending) {
            call_once(callbacks.copy_terrain_layers);
        } else {
            state.terrain_copy_pending = false;
        }
        if (state.terrain_mode_active) {
            call_once(callbacks.process_terrain_entries);
        }
    }
    call_once(callbacks.rebuild_collision_map);
    if (state.scene_state > 1U) {
        call_once(callbacks.load_scene_resources);
    }
    call_once(callbacks.clear_scene_transitions);
    call_once(callbacks.populate_entities);
    call_once(callbacks.dispatch_entity_actions);
    if (state.compressed_entity_data_pending) {
        state.compressed_entity_data_pending = false;
        call_once(callbacks.decode_entity_data);
    }
    call_once(callbacks.mark_entities_for_render);
}

void update_entity_frame(std::array<EntityFrameRecord, 16>& entities,
                         const EntityFrameCallbacks& callbacks) {
    if (callbacks.sort_by_depth) {
        callbacks.sort_by_depth(entities);
    }
    for (auto& entity : entities) {
        if (!entity.active) {
            continue;
        }
        if (entity.timer == 0) {
            if (entity.behavior == 6U) {
                if (callbacks.behavior_six) callbacks.behavior_six(entity);
            } else if (entity.behavior == 0x0DU) {
                if (callbacks.behavior_thirteen) callbacks.behavior_thirteen(entity);
            }
            continue;
        }
        if (callbacks.behavior_table && entity.behavior != 0U) {
            callbacks.behavior_table(entity);
        }
        if ((entity.flags & 0x20U) != 0U && callbacks.secondary_timer) {
            callbacks.secondary_timer(entity);
        }
        if ((entity.flags & 0x80U) != 0U && callbacks.primary_timer) {
            callbacks.primary_timer(entity);
        }
        if ((entity.flags & 0x10U) != 0U && callbacks.delayed_effect) {
            callbacks.delayed_effect(entity);
        }
        if ((entity.flags & 0x40U) != 0U && callbacks.animated_effect) {
            callbacks.animated_effect(entity);
        }
        if ((entity.flags & 0x80U) != 0U && callbacks.sprite_burst) {
            callbacks.sprite_burst(entity);
        }
        if (callbacks.expand_sprites) {
            callbacks.expand_sprites(entity);
        }
    }
    call_once(callbacks.finish_effect);
    call_once(callbacks.finish_wave);
    call_once(callbacks.finish_post_state);
}

void refresh_projected_entity_sprites(
    const std::array<EntityFrameRecord, 16>& entities,
    const std::function<void(const EntityFrameRecord&)>& project_entity,
    const std::function<void(const EntityFrameRecord&)>& build_sprite) {
    for (const auto& entity : entities) {
        if (!entity.active) {
            continue;
        }
        if (project_entity) project_entity(entity);
        if (entity.mode != 0U && build_sprite) build_sprite(entity);
    }
}

std::uint32_t select_projected_entity_sprite_source(const std::int16_t type) {
    if (type < 0) {
        if ((static_cast<std::uint16_t>(type) & 0xFF00U) == 0xC000U) {
            return 0x00005F82U;
        }
        return 0x00005FA0U;
    }
    const auto index = static_cast<std::uint16_t>(type);
    if (index < 0x63U) {
        if (index < kProjectedEntitySpriteSources.size()) {
            return kProjectedEntitySpriteSources[index];
        }
        return 0U;
    }
    if (index == 0x64U) {
        return 0x00005FAAU;
    }
    return 0x0001D47CU;
}

namespace {

std::vector<SpriteSourceRecord> projected_entity_sprite_records(
    const std::uint32_t source) {
    switch (source) {
    case 0x00005F78U:
        return {{-8, -5, 4, 0x07F2U}};
    case 0x000095A4U:
        return {{-4, -4, 0, 0x05B2U}};
    case 0x00005F96U:
        return {{-5, -5, 4, 0x07F6U}};
    case 0x00005DEAU:
        return {{-14, 12, 0, 0x05E7U}};
    case 0x00005F8CU:
        return {{-8, -10, 4, 0x07F0U}};
    case 0x00005FA0U:
        return {{-3, -6, 1, 0x07F4U}};
    case 0x00005FAAU:
        return {{-3, -9, 1, 0x05E3U}};
    case 0x00005F82U:
        return {{-4, -5, 1, 0x07F8U}};
    default:
        return {};
    }
}

} // namespace

std::vector<BuiltSpriteRecord> refresh_projected_entity_sprites(
    std::array<ProjectedEntitySpriteSlot, 10>& slots,
    const std::uint16_t camera_x,
    const std::uint16_t camera_y,
    const std::uint8_t first_sequence) {
    std::vector<BuiltSpriteRecord> records;
    auto sequence = first_sequence;
    for (auto& slot : slots) {
        if ((slot.flags & 0x01U) == 0U) {
            continue;
        }
        EntityProjectionState projection{
            slot.world_x, slot.world_y, slot.sprite_offset_y, slot.screen_x,
            slot.screen_y};
        project_entity_screen_position_standard(projection, camera_x, camera_y);
        slot.screen_x = projection.screen_x;
        slot.screen_y = projection.screen_y;
        if (slot.screen_y == 0U) {
            continue;
        }
        const auto built = build_title_sprite_records(
            projected_entity_sprite_records(
                select_projected_entity_sprite_source(slot.type)),
            static_cast<std::int16_t>(slot.screen_x),
            static_cast<std::int16_t>(slot.screen_y), sequence, 0x4000U);
        sequence = static_cast<std::uint8_t>(sequence + built.size());
        records.insert(records.end(), built.begin(), built.end());
    }
    return records;
}

void spawn_entity_effect(
    EntityEffectSlotState& effect,
    const EntityEffectSource& source,
    const std::uint16_t slot_index,
    const std::function<std::uint16_t(std::uint16_t, std::uint16_t)>&
        project_tile,
    const std::function<void(const EntityEffectSlotState&)>& append_sprite,
    const std::function<void()>& service_audio) {
    effect.active = true;
    effect.source_entity = source.x;
    effect.variant = source.item_code;
    effect.countdown = 0;
    effect.phase = static_cast<std::uint8_t>(source.heading >> 13U);
    effect.tile_index = project_tile ? project_tile(source.x, source.y) : 0U;
    effect.source_entity = slot_index;
    if (append_sprite) {
        append_sprite(effect);
    }
    call_once(service_audio);
}

void advance_entity_effect_timer(
    EntityEffectSlotState& effect,
    const std::function<void()>& clear_slot,
    const std::function<void()>& restore_tiles,
    const std::function<void()>& project_entity,
    const std::function<void(std::uint8_t)>& build_sprite_records,
    const std::function<void(const EntityEffectSlotState&)>& append_sprite) {
    if (!effect.active) {
        return;
    }
    if (--effect.countdown < 0) {
        effect.active = false;
        call_once(clear_slot);
        call_once(restore_tiles);
        return;
    }
    call_once(project_entity);
    for (std::uint8_t index = 0; index < 4U; ++index) {
        if (build_sprite_records) {
            build_sprite_records(index);
        }
    }
    if (append_sprite) {
        append_sprite(effect);
    }
}

std::vector<BuiltSpriteRecord> advance_entity_effect_timer(
    EntityEffectTimerState& state,
    std::array<std::uint8_t, 18>& slots,
    std::vector<std::uint8_t>& collision_map,
    const std::uint16_t camera_x,
    const std::uint16_t camera_y,
    const std::uint8_t first_sequence) {
    --state.countdown;
    if (state.countdown < 0) {
        EffectRecordState record{};
        record.heading = state.heading;
        record.slot = state.slot;
        record.collision_base = state.collision_base;
        record.tile_bytes = state.saved_tiles;
        record.coordinate = 1;
        reset_effect_record(record, slots, collision_map);
        return {};
    }
    EntityProjectionState projection{
        state.world_x, state.world_y, state.sprite_offset_y, state.screen_x,
        state.screen_y};
    project_entity_screen_position_standard(projection, camera_x, camera_y);
    state.screen_x = projection.screen_x;
    state.screen_y = projection.screen_y;
    const auto remainder = static_cast<std::uint16_t>(
        static_cast<std::uint16_t>(state.countdown) % 5U);
    const auto next = static_cast<std::uint16_t>(
        remainder + 1U == 5U ? 0U : remainder + 1U);
    const auto heading =
        state.heading < kEffectTimerHeadingOffsets.size() ? state.heading : 0U;
    const auto& pairs = kEffectTimerHeadingOffsets[heading];
    const auto attributes =
        kEffectTimerSpriteAttributes[static_cast<std::uint16_t>(state.countdown) &
                                     3U];
    const std::vector<SpriteSourceRecord> source{{-16, -16, 15, attributes}};
    std::vector<BuiltSpriteRecord> records;
    auto sequence = first_sequence;
    for (int step = 4; step >= 0; --step) {
        if (step != static_cast<int>(remainder) &&
            step != static_cast<int>(next)) {
            continue;
        }
        const auto& pair = pairs[static_cast<std::size_t>(4 - step)];
        const auto built = build_title_sprite_records_with_attribute_offset(
            source,
            static_cast<std::int16_t>(
                static_cast<std::int16_t>(state.screen_x) + pair.first),
            static_cast<std::int16_t>(
                static_cast<std::int16_t>(state.screen_y) + pair.second),
            sequence, 0x2000U, state.attribute_offset);
        sequence = static_cast<std::uint8_t>(sequence + built.size());
        records.insert(records.end(), built.begin(), built.end());
    }
    return records;
}

std::uint16_t effect_animation_heading(const std::uint16_t facing) {
    auto rotated = static_cast<std::uint16_t>(
        (facing << 3U) | (facing >> 13U));
    if ((rotated & 0x8000U) != 0U) {
        ++rotated;
    }
    return static_cast<std::uint16_t>(rotated & 7U);
}

std::vector<BuiltSpriteRecord> advance_effect_animation(
    EffectAnimationSpriteState& state,
    std::array<std::uint8_t, 18>& slots,
    const std::uint8_t first_sequence) {
    --state.animation.countdown;
    if (state.animation.countdown < 0) {
        state.animation.countdown = 6;
        ++state.animation.phase;
        if (state.animation.phase >= 4U) {
            clear_effect_slot(slots, state.slot);
            state.entity_flags = static_cast<std::uint8_t>(
                state.entity_flags & ~0x40U);
            state.animation.active = false;
            return {};
        }
    }
    const auto heading = effect_animation_heading(state.facing);
    const auto phase =
        state.animation.phase < 4U ? state.animation.phase : 0U;
    const auto& origin = kEffectAnimationHeadingOrigins[heading];
    const auto& sprite = kEffectAnimationSprites[heading][phase];
    return build_title_sprite_records_with_attribute_offset(
        {sprite},
        static_cast<std::int16_t>(
            static_cast<std::int16_t>(state.screen_x) + origin.first),
        static_cast<std::int16_t>(
            static_cast<std::int16_t>(state.screen_y) + origin.second),
        first_sequence, 0x2000U, state.attribute_offset);
}

bool advance_effect_animation(
    EffectAnimationState& state,
    const std::function<void()>& clear_slot,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprite_records) {
    if (!state.active) {
        return false;
    }
    if (--state.countdown < 0) {
        state.countdown = 6;
        ++state.phase;
        if (state.phase > state.phase_limit) {
            state.active = false;
            call_once(clear_slot);
            return false;
        }
        call_once(append_sprite);
    }
    call_once(build_sprite_records);
    return true;
}

bool advance_effect_animation_variant(
    EffectAnimationState& state,
    const std::function<void()>& clear_slot,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprite_records) {
    state.phase_limit = 4U;
    return advance_effect_animation(state, clear_slot, append_sprite,
                                    build_sprite_records);
}

std::vector<BuiltSpriteRecord> advance_effect_animation_variant(
    EffectAnimationVariantState& state,
    const std::uint8_t first_sequence) {
    --state.animation.countdown;
    if (state.animation.countdown < 0) {
        state.animation.countdown = 6;
        ++state.animation.phase;
    }
    if (state.animation.phase >= 5U) {
        state.auxiliary = 0;
        state.animation.active = false;
        return {};
    }
    auto y = static_cast<std::int16_t>(state.screen_x);
    auto x = static_cast<std::int16_t>(state.screen_y);
    if (state.animation.phase >= 4U) {
        x = static_cast<std::int16_t>(
            x + static_cast<std::int16_t>((state.animation.phase - 4U) * 4U));
        y = static_cast<std::int16_t>(
            y + static_cast<std::int16_t>(state.jitter_bit & 1U));
    }
    return build_title_sprite_records_with_attribute_offset(
        {kEffectAnimationVariantSprites[state.animation.phase]}, y, x,
        first_sequence, 0x6000U, state.attribute_offset);
}

void run_effect_variant_a(
    EntityEffectSlotState& effect,
    const std::function<void()>& clone_variant,
    const std::function<void()>& prepare_auxiliary,
    const std::function<void()>& clear_slot) {
    if (!effect.active) {
        return;
    }
    if (effect.countdown > 0) {
        --effect.countdown;
        return;
    }
    call_once(clone_variant);
    call_once(prepare_auxiliary);
    if (effect.phase > 4U) {
        effect.active = false;
        call_once(clear_slot);
    }
}

void run_effect_variant_b(
    EntityEffectSlotState& effect,
    const std::function<void()>& clone_variant,
    const std::function<void()>& prepare_auxiliary,
    const std::function<void()>& clear_slot) {
    if (!effect.active) {
        return;
    }
    if (effect.countdown > 0) {
        --effect.countdown;
        return;
    }
    call_once(clone_variant);
    call_once(prepare_auxiliary);
    if (effect.phase > 3U) {
        effect.active = false;
        call_once(clear_slot);
    }
}

void advance_entity_animation_effect(
    EffectAnimationState& state,
    const std::function<void()>& build_special_sprite,
    const std::function<void()>& build_sprite_records) {
    if (!state.active) {
        return;
    }
    if (--state.countdown < 0) {
        state.countdown = 3;
        state.phase = static_cast<std::uint8_t>((state.phase + 1U) & 3U);
        call_once(build_special_sprite);
    }
    call_once(build_sprite_records);
}

void refresh_delayed_effect_sprites(
    EntityEffectSlotState& effect,
    const std::function<void()>& build_sprite_records,
    const std::function<void()>& append_sprite) {
    if (!effect.active) {
        return;
    }
    if (--effect.countdown < 0) {
        effect.countdown = 0;
        for (int index = 0; index < 4; ++index) {
            call_once(build_sprite_records);
        }
    }
    call_once(append_sprite);
}

bool set_sprite_descriptor_carry() { return true; }

void advance_jitter_effect(
    JitterEffectState& state,
    const std::function<std::uint32_t(std::uint32_t&)>& next_random,
    const std::function<void()>& append_sprite,
    const std::function<void()>& build_sprite_records) {
    if (!state.animation.active) {
        return;
    }
    const auto ticked = state.animation.countdown-- < 0;
    if (ticked) {
        state.animation.countdown = 6;
        ++state.animation.phase;
    }
    if (state.animation.phase > state.animation.phase_limit) {
        --state.remaining;
        if (state.remaining < 0) {
            state.behavior = 0;
            state.animation.active = false;
            return;
        }
        const auto random = next_random ? next_random(state.random_state) : 0U;
        state.offset_x = static_cast<std::int16_t>(random & 7U);
        if ((random & 2U) != 0U) state.offset_x = -state.offset_x;
        state.offset_y = static_cast<std::int16_t>((random >> 2U) & 7U);
        if ((random & 4U) != 0U) state.offset_y = -state.offset_y;
        state.animation.phase = 0;
        state.animation.countdown = 5;
    }
    if (state.offset_x != 0 || state.offset_y != 0) {
        call_once(append_sprite);
    }
    call_once(build_sprite_records);
}

} // namespace shadowrun::ghidra_conversion
