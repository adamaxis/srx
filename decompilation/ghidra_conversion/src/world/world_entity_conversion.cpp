#include "shadowrun_ghidra/world/world_entity_conversion.hpp"

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
