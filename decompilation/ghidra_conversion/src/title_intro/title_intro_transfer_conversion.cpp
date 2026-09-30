#include "shadowrun_ghidra/title_intro/title_intro_transfer_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void clear_title_intro_display_buffer(
    const std::function<void(const TitleIntroDisplayClearRequest&)>& clear) {
    if (clear) {
        clear({});
    }
}

void fill_title_intro_word_block(
    const TitleIntroWordFillPlan& plan,
    const std::function<void(const TitleIntroFillRowRequest&)>& write_row) {
    if (!write_row) {
        return;
    }
    auto destination = plan.destination_offset;
    for (std::uint16_t row = 0; row < plan.row_count; ++row) {
        write_row({destination, plan.word_count, plan.fill_word, row});
        destination = static_cast<std::uint16_t>(destination + plan.row_stride);
    }
}

void reset_world_frame_service_state(WorldFrameServiceState& state) {
    state.words.fill(0U);
    state.words[0] = 0x11C0U;
    state.words[2] = 0xFE80U;
    state.words[0x11] = 0xFFFFU;
    state.words[0x12] = 0x00B4U;
}

WorldSceneEffectPhaseResult advance_world_scene_effect_phase(
    WorldFrameServiceState& state,
    const WorldSceneEffectPhaseCallbacks& callbacks) {
    const auto phase = static_cast<std::int16_t>(state.words[0x12]);
    if (phase > 0x32) {
        return {};
    }

    WorldSceneEffectPhaseAction action =
        WorldSceneEffectPhaseAction::load_default_resource_metadata;
    if (phase == 0x14) {
        action = WorldSceneEffectPhaseAction::play_special_effect_audio;
        if (callbacks.play_audio) {
            callbacks.play_audio(WorldSceneEffectAudioCue::special_effect);
        }
        if (callbacks.spawn_special_effect) {
            callbacks.spawn_special_effect();
        }
    } else if (phase == 0x16 || phase == 0x20 || phase == 0x21 ||
               phase == 0x23) {
        action = WorldSceneEffectPhaseAction::load_transition_resource_metadata;
        if (callbacks.copy_resource_metadata) {
            callbacks.copy_resource_metadata(
                WorldSceneEffectMetadataSource::transition_blank_resource);
        }
    } else if (phase == 0x18 || phase == 0x1A || phase == 0x1D ||
               phase == 0x24 || phase == 0x27) {
        action = WorldSceneEffectPhaseAction::shift_scene_origin_backward;
        state.scene_origin_x = static_cast<std::int16_t>(
            state.scene_origin_x - 8);
        state.scene_origin_y = static_cast<std::int16_t>(
            state.scene_origin_y - 8);
    } else if (phase == 0x19 || phase == 0x1B || phase == 0x1E ||
               phase == 0x25 || phase == 0x26 || phase == 0x28 ||
               phase == 0x29) {
        action = WorldSceneEffectPhaseAction::play_scene_transition_audio;
        if (callbacks.play_audio) {
            callbacks.play_audio(WorldSceneEffectAudioCue::scene_transition);
        }
        state.scene_origin_x = static_cast<std::int16_t>(
            state.scene_origin_x + 8);
        state.scene_origin_y = static_cast<std::int16_t>(
            state.scene_origin_y + 8);
    } else {
        if (callbacks.copy_resource_metadata) {
            callbacks.copy_resource_metadata(
                WorldSceneEffectMetadataSource::default_mode_resource);
        }
    }

    state.dirty_flags = static_cast<std::uint8_t>(state.dirty_flags | 0x0DU);
    return {action, state.dirty_flags};
}

void spawn_world_scene_secondary_effect(
    WorldSceneSecondaryEffectState& state,
    const WorldSceneSecondaryEffectCallbacks& callbacks) {
    state.behavior_flags = static_cast<std::uint8_t>(
        state.behavior_flags | 0x01U);
    if (callbacks.apply_entity_secondary_effect) {
        callbacks.apply_entity_secondary_effect();
    }
    if (callbacks.prepare_auxiliary_behavior) {
        callbacks.prepare_auxiliary_behavior();
    }
}

namespace {

using WorldSceneEffectSourceList = std::vector<SpriteSourceRecord>;

// ROM 0x1FEA2E: the primary timer selects one of these complete source
// lists. Each list is the data consumed by one 0x1378 call.
const std::array<WorldSceneEffectSourceList, 9> kWorldSceneEffectTimerASources{{
    {{ {-13, -6, 0x0B, 0x0381U}, {-13, -16, 0x09, 0x038DU},
       {-7, 26, 0x04, 0x0393U} }},
    {{ {-13, -6, 0x0B, 0x0381U}, {-13, -16, 0x09, 0x038DU},
       {-7, 26, 0x04, 0x0393U} }},
    {{ {-13, -6, 0x0B, 0x0381U}, {-13, -16, 0x09, 0x038DU},
       {-7, 26, 0x04, 0x0393U} }},
    {{ {-13, -6, 0x0B, 0x0381U}, {-13, -16, 0x09, 0x038DU},
       {-7, 26, 0x04, 0x0393U} }},
    {{ {-13, -6, 0x0B, 0x0415U}, {-13, -16, 0x09, 0x0421U},
       {-7, 26, 0x04, 0x0427U} }},
    {{ {-13, -6, 0x0B, 0x0381U}, {-13, -16, 0x09, 0x038DU},
       {-7, 26, 0x04, 0x0393U} }},
    {{ {-13, -6, 0x0B, 0x0415U}, {-13, -16, 0x09, 0x0421U},
       {-7, 26, 0x04, 0x0427U} }},
    {{ {-13, -6, 0x0B, 0x0381U}, {-13, -16, 0x09, 0x038DU},
       {-7, 26, 0x04, 0x0393U} }},
    {{ {-13, -6, 0x0B, 0x0415U}, {-13, -16, 0x09, 0x0421U},
       {-7, 26, 0x04, 0x0427U} }},
}};

// ROM 0x1FE926: the secondary timer's ten source-list pointers resolve to
// these two-record lists.
const std::array<WorldSceneEffectSourceList, 10> kWorldSceneEffectTimerBSources{{
    {{ {-15, -16, 0x0F, 0x034EU}, {-1, 16, 0x00, 0x035EU} }},
    {{ {-13, -16, 0x0F, 0x035FU}, {-1, 16, 0x00, 0x036FU} }},
    {{ {-13, -16, 0x0F, 0x0370U}, {-2, 16, 0x00, 0x0380U} }},
    {{ {-19, -16, 0x0F, 0x0B5FU}, {-7, 16, 0x00, 0x0B6FU} }},
    {{ {-17, -16, 0x0F, 0x0B4EU}, {-7, 16, 0x00, 0x0B5EU} }},
    {{ {-17, -16, 0x0F, 0x0B4EU}, {-7, 16, 0x00, 0x0B5EU} }},
    {{ {-19, -16, 0x0F, 0x0B5FU}, {-7, 16, 0x00, 0x0B6FU} }},
    {{ {-13, -16, 0x0F, 0x0370U}, {-2, 16, 0x00, 0x0380U} }},
    {{ {-13, -16, 0x0F, 0x035FU}, {-1, 16, 0x00, 0x036FU} }},
    {{ {-15, -16, 0x0F, 0x034EU}, {-1, 16, 0x00, 0x035EU} }},
}};

// ROM 0x1FE946: the tertiary timer includes deliberate empty entries at the
// source's 0x1D47C sentinel list. Preserve those empty animation phases as
// empty typed lists rather than manufacturing placeholder sprites.
const std::array<WorldSceneEffectSourceList, 40> kWorldSceneEffectTimerCSources{{
    {{ {-15, -52, 0x09, 0x040FU}, {-49, -52, 0x09, 0x0C0FU} }},
    {{ {-15, -47, 0x0F, 0x03FCU}, {4, -15, 0x01, 0x040CU},
       {18, -44, 0x00, 0x040EU}, {-57, -47, 0x0F, 0x0BFCU},
       {-52, -15, 0x01, 0x0C0CU}, {-66, -44, 0x00, 0x0C0EU} }},
    {{ {-15, -47, 0x0F, 0x03E1U}, {-1, -15, 0x0A, 0x03F1U},
       {17, -46, 0x04, 0x03FAU}, {-57, -47, 0x0F, 0x0BE1U},
       {-63, -15, 0x0A, 0x0BF1U}, {-73, -46, 0x04, 0x0BFAU} }},
    {{ {-14, -51, 0x0F, 0x03B0U}, {-1, -19, 0x0F, 0x03C0U},
       {18, -50, 0x0F, 0x03D0U}, {23, 13, 0x00, 0x03E0U},
       {-58, -51, 0x0F, 0x0BB0U}, {-71, -19, 0x0F, 0x0BC0U},
       {-90, -50, 0x0F, 0x0BD0U}, {-71, 13, 0x00, 0x0BE0U} }},
    {}, {}, {}, {}, {}, {}, {}, {}, {}, {},
    {{ {-10, -56, 0x0A, 0x03A7U} }},
    {{ {-10, -56, 0x0A, 0x03A7U} }},
    {{ {-11, -62, 0x0B, 0x039BU} }},
    {{ {-11, -62, 0x0B, 0x039BU} }},
    {{ {-10, -61, 0x09, 0x0395U} }},
    {{ {-10, -61, 0x09, 0x0395U} }},
    {}, {}, {}, {}, {}, {}, {},
    {{ {-52, -56, 0x0A, 0x0BA7U} }},
    {{ {-52, -56, 0x0A, 0x0BA7U} }},
    {{ {-51, -62, 0x0B, 0x0B9BU} }},
    {{ {-51, -62, 0x0B, 0x0B9BU} }},
    {{ {-52, -61, 0x09, 0x0B95U} }},
    {{ {-52, -61, 0x09, 0x0B95U} }},
    {}, {}, {}, {}, {}, {}, {},
}};

// ROM 0x1FEA0E: the refresh leaf selects one of these complete lists using
// the shared scene frame and publishes them at the raw projected origin.
const std::array<WorldSceneEffectSourceList, 8>
    kWorldSceneEffectRefreshSources{{
        {{ {-16, -5, 0x00, 0x0336U}, {-14, -20, 0x00, 0x0346U} }},
        {{ {-14, -8, 0x0F, 0x031AU}, {-14, -20, 0x0D, 0x032AU},
           {-8, 24, 0x0C, 0x0332U} }},
        {{ {-9, -17, 0x0F, 0x02FEU}, {-9, -20, 0x0C, 0x030EU},
           {-9, 15, 0x0D, 0x0312U} }},
        {{ {-8, -20, 0x0F, 0x02E2U}, {-4, 12, 0x0E, 0x02F2U} }},
        {{ {-8, -5, 0x0F, 0x02C9U}, {-7, -20, 0x0D, 0x02D9U},
           {12, 27, 0x00, 0x02E1U} }},
        {{ {-8, -20, 0x0F, 0x02E2U}, {-4, 12, 0x0E, 0x02F2U} }},
        {{ {-9, -17, 0x0F, 0x02FEU}, {-9, -20, 0x0C, 0x030EU},
           {-9, 15, 0x0D, 0x0312U} }},
        {{ {-14, -8, 0x0F, 0x031AU}, {-14, -20, 0x0D, 0x032AU},
           {-8, 24, 0x0C, 0x0332U} }},
    }};

void tick_world_scene_effect_timer(
    std::uint16_t& countdown,
    std::uint16_t& frame,
    const std::int16_t reload,
    const std::uint16_t wrap) {
    auto remaining = static_cast<std::int16_t>(countdown);
    --remaining;
    if (remaining < 0) {
        remaining = reload;
        auto next = static_cast<std::uint16_t>(frame + 1U);
        if (next >= wrap) {
            next = 0U;
        }
        frame = next;
    }
    countdown = static_cast<std::uint16_t>(remaining);
}

WorldSceneEffectSpriteBatch build_world_scene_effect_sprite_batch(
    const WorldFrameServiceState& state,
    const std::vector<SpriteSourceRecord>& source,
    const std::int16_t extra_x,
    const std::int16_t extra_y,
    const std::uint16_t frame_index,
    const std::uint8_t first_sequence,
    const bool apply_heading_offsets) {
    WorldSceneEffectSpriteBatch batch{};
    batch.frame_index = frame_index;
    batch.first_sequence = first_sequence;
    batch.next_sequence = first_sequence;
    const auto heading =
        static_cast<std::size_t>(state.words[0x0C] %
                                 kWorldSceneEffectHeadingOffsets.size());
    const auto heading_x = apply_heading_offsets
                               ? kWorldSceneEffectHeadingOffsets[heading][0]
                               : 0;
    const auto heading_y = apply_heading_offsets
                               ? kWorldSceneEffectHeadingOffsets[heading][1]
                               : 0;
    const auto origin_x = static_cast<std::int16_t>(
        static_cast<std::int16_t>(state.words[5]) + heading_x + extra_x);
    const auto origin_y = static_cast<std::int16_t>(
        static_cast<std::int16_t>(state.words[6]) + heading_y + extra_y);
    batch.records = build_title_sprite_records(
        source, origin_y, origin_x, first_sequence, 0x4000U);
    batch.next_sequence = static_cast<std::uint8_t>(
        first_sequence + batch.records.size());
    return batch;
}

} // namespace

WorldSceneEffectSpriteBatch advance_world_scene_effect_timer_a(
    WorldFrameServiceState& state,
    const std::uint8_t first_sequence) {
    tick_world_scene_effect_timer(state.words[3], state.words[0x0B], 0x14, 9U);
    const auto frame = static_cast<std::size_t>(state.words[0x0B]);
    if (frame >= kWorldSceneEffectTimerASources.size()) {
        return {static_cast<std::uint16_t>(frame), first_sequence,
                first_sequence, {}};
    }
    return build_world_scene_effect_sprite_batch(
        state, kWorldSceneEffectTimerASources[frame], 0, 0,
        static_cast<std::uint16_t>(frame), first_sequence, true);
}

void advance_world_scene_effect_frame(WorldFrameServiceState& state) {
    tick_world_scene_effect_timer(state.words[1], state.words[0x0C], 0x0A,
                                  8U);
}

WorldSceneEffectSpriteBatch advance_world_scene_effect_timer_b(
    WorldFrameServiceState& state,
    const std::uint8_t first_sequence) {
    tick_world_scene_effect_timer(
        state.words[0x0D], state.words[0x0E], 0x0A, 10U);
    const auto frame = static_cast<std::size_t>(state.words[0x0E]);
    if (frame >= kWorldSceneEffectTimerBSources.size()) {
        return {static_cast<std::uint16_t>(frame), first_sequence,
                first_sequence, {}};
    }
    return build_world_scene_effect_sprite_batch(
        state, kWorldSceneEffectTimerBSources[frame], 0, -0x18,
        static_cast<std::uint16_t>(frame), first_sequence, true);
}

WorldSceneEffectSpriteBatch advance_world_scene_effect_timer_c(
    WorldFrameServiceState& state,
    const std::uint8_t first_sequence) {
    tick_world_scene_effect_timer(
        state.words[0x10], state.words[0x0F], 0x04, 0x28U);
    const auto frame = static_cast<std::size_t>(state.words[0x0F]);
    if (frame >= kWorldSceneEffectTimerCSources.size()) {
        return {static_cast<std::uint16_t>(frame), first_sequence,
                first_sequence, {}};
    }
    return build_world_scene_effect_sprite_batch(
        state, kWorldSceneEffectTimerCSources[frame], 0x12, 0x30,
        static_cast<std::uint16_t>(frame), first_sequence, true);
}

WorldSceneEffectSpriteBatch refresh_world_scene_effect_sprites(
    const WorldFrameServiceState& state,
    const std::uint8_t first_sequence) {
    const auto frame = static_cast<std::size_t>(state.words[0x0C]);
    if (frame >= kWorldSceneEffectRefreshSources.size()) {
        return {static_cast<std::uint16_t>(frame), first_sequence,
                first_sequence, {}};
    }
    return build_world_scene_effect_sprite_batch(
        state, kWorldSceneEffectRefreshSources[frame], 0, 0,
        static_cast<std::uint16_t>(frame), first_sequence, false);
}

namespace {

void publish_world_scene_effect_sprites(
    const WorldSceneEffectSpriteBatch& batch,
    WorldSceneEntityEffectRuntimeState& runtime_state,
    const WorldSceneEntityEffectCallbacks& callbacks) {
    if (callbacks.publish_sprites) {
        callbacks.publish_sprites(batch);
    }
    runtime_state.sprite_sequence = batch.next_sequence;
}

void refresh_world_scene_effect_sprites_for_runtime(
    const WorldFrameServiceState& frame_state,
    WorldSceneEntityEffectRuntimeState& runtime_state,
    const WorldSceneEntityEffectCallbacks& callbacks) {
    const auto batch = refresh_world_scene_effect_sprites(
        frame_state, runtime_state.sprite_sequence);
    if (callbacks.refresh_world_scene_effect_sprites) {
        callbacks.refresh_world_scene_effect_sprites(batch);
    }
    runtime_state.sprite_sequence = batch.next_sequence;
}

void spawn_world_scene_secondary_effect_for_slot(
    WorldSceneEffectEntitySlot& slot,
    const WorldSceneEntityEffectCallbacks& callbacks) {
    // This is the record-level part of 0x5D3E6. Keep the callback order
    // identical to spawn_world_scene_secondary_effect(), while exposing the
    // selected slot rather than an emulated A1 register.
    slot.behavior_flags = static_cast<std::uint8_t>(
        slot.behavior_flags | 0x01U);
    if (callbacks.apply_entity_secondary_effect) {
        callbacks.apply_entity_secondary_effect(slot);
    }
    if (callbacks.prepare_entity_auxiliary_behavior) {
        callbacks.prepare_entity_auxiliary_behavior(slot);
    }
}

} // namespace

WorldSceneEntityEffectResult advance_world_scene_entity_effect(
    WorldFrameServiceState& frame_state,
    WorldSceneEntityEffectRuntimeState& runtime_state,
    const WorldSceneEntityEffectCallbacks& callbacks) {
    if (runtime_state.scene_mode != 2U && runtime_state.scene_mode != 3U) {
        return {};
    }

    const auto stage = static_cast<std::int16_t>(frame_state.words[0x11]);
    if (stage < 0) {
        return {};
    }

    if (stage == 2) {
        if (callbacks.project_entity_screen_position) {
            callbacks.project_entity_screen_position();
        }
        advance_world_scene_effect_frame(frame_state);
        publish_world_scene_effect_sprites(
            advance_world_scene_effect_timer_c(
                frame_state, runtime_state.sprite_sequence),
            runtime_state, callbacks);
        publish_world_scene_effect_sprites(
            advance_world_scene_effect_timer_b(
                frame_state, runtime_state.sprite_sequence),
            runtime_state, callbacks);
        publish_world_scene_effect_sprites(
            advance_world_scene_effect_timer_a(
                frame_state, runtime_state.sprite_sequence),
            runtime_state, callbacks);
        refresh_world_scene_effect_sprites_for_runtime(
            frame_state, runtime_state, callbacks);

        if (frame_state.words[0x0C] != 3U) {
            return {WorldSceneEntityEffectAction::rendered_active_effect,
                    std::nullopt};
        }

        for (std::size_t index = 0;
             index < runtime_state.candidate_entities.size(); ++index) {
            auto& entity = runtime_state.candidate_entities[index];
            if ((entity.record_flags & 0x01U) == 0U ||
                (entity.record_flags & 0x10U) != 0U ||
                (entity.record_flags & 0x40U) != 0U ||
                entity.secondary_effect_trigger == 0U) {
                continue;
            }
            spawn_world_scene_secondary_effect_for_slot(entity, callbacks);
            return {WorldSceneEntityEffectAction::
                        selected_existing_entity_effect,
                    index};
        }

        auto scene_timer = static_cast<std::int16_t>(
            frame_state.words[0x12]);
        --scene_timer;
        frame_state.words[0x12] = static_cast<std::uint16_t>(scene_timer);
        if (scene_timer < 0) {
            if (callbacks.begin_world_activity_if_idle) {
                callbacks.begin_world_activity_if_idle();
            }
            return {WorldSceneEntityEffectAction::completed_world_activity,
                    std::nullopt};
        }

        runtime_state.terminal_auxiliary_marker = true;
        if (callbacks.prepare_terminal_auxiliary_entity) {
            callbacks.prepare_terminal_auxiliary_entity();
        }
        spawn_world_scene_secondary_effect_for_slot(
            runtime_state.auxiliary_entity, callbacks);
        runtime_state.auxiliary_entity.secondary_effect_trigger = 4U;
        runtime_state.auxiliary_entity.secondary_effect_phase = 4U;
        runtime_state.auxiliary_entity.record_flags = 0x21U;
        return {WorldSceneEntityEffectAction::
                    spawned_terminal_auxiliary_effect,
                std::nullopt};
    }

    if (stage == 0) {
        auto scene_timer = static_cast<std::int16_t>(
            frame_state.words[0x12]);
        --scene_timer;
        frame_state.words[0x12] = static_cast<std::uint16_t>(scene_timer);
        if (scene_timer >= 0) {
            advance_world_scene_effect_phase(frame_state,
                                             callbacks.phase_callbacks);
            return {WorldSceneEntityEffectAction::advanced_resource_phase,
                    std::nullopt};
        }

        if (callbacks.install_scene_resource_index_entries) {
            callbacks.install_scene_resource_index_entries();
        }
        frame_state.words[0x11] = 1U;
        frame_state.words[0x12] = 0x0100U;
        return {WorldSceneEntityEffectAction::
                    entered_active_resource_stage,
                std::nullopt};
    }

    const auto origin_y = static_cast<std::int16_t>(frame_state.words[2]);
    if (origin_y < 0 || frame_state.words[2] < 0x0248U) {
        frame_state.words[2] = static_cast<std::uint16_t>(
            frame_state.words[2] + 8U);
    }
    if (callbacks.project_entity_screen_position) {
        callbacks.project_entity_screen_position();
    }
    advance_world_scene_effect_frame(frame_state);
    publish_world_scene_effect_sprites(
        advance_world_scene_effect_timer_b(
            frame_state, runtime_state.sprite_sequence),
        runtime_state, callbacks);
    publish_world_scene_effect_sprites(
        advance_world_scene_effect_timer_a(
            frame_state, runtime_state.sprite_sequence),
        runtime_state, callbacks);
    refresh_world_scene_effect_sprites_for_runtime(
        frame_state, runtime_state, callbacks);

    auto scene_timer = static_cast<std::int16_t>(frame_state.words[0x12]);
    --scene_timer;
    frame_state.words[0x12] = static_cast<std::uint16_t>(scene_timer);
    if (scene_timer < 0) {
        frame_state.words[0x11] = 2U;
        frame_state.words[0x12] = 0x0028U;
        return {WorldSceneEntityEffectAction::entered_final_effect_stage,
                std::nullopt};
    }
    return {WorldSceneEntityEffectAction::rendered_active_effect,
            std::nullopt};
}

std::vector<BuiltSpriteRecord> render_title_intro_scene_entities(
    const std::int16_t y_offset,
    const std::int16_t x_offset,
    const std::uint8_t first_sequence) {
    return build_title_sprite_records(
        {kTitleIntroSceneEntitySource.begin(),
         kTitleIntroSceneEntitySource.end()},
        y_offset, x_offset, first_sequence, 0x2000U);
}

} // namespace shadowrun::ghidra_conversion
