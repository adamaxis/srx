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

WorldSceneEffectSpriteBuild build_world_scene_effect_sprite(
    const WorldFrameServiceState& state,
    const std::int16_t extra_x,
    const std::int16_t extra_y,
    const std::uint32_t source) {
    WorldSceneEffectSpriteBuild build{};
    const auto heading =
        static_cast<std::size_t>(state.words[0x0C] %
                                 kWorldSceneEffectHeadingOffsets.size());
    const auto origin_x = static_cast<std::int16_t>(state.words[5]);
    const auto origin_y = static_cast<std::int16_t>(state.words[6]);
    build.x = static_cast<std::int16_t>(
        origin_x + kWorldSceneEffectHeadingOffsets[heading][0] + extra_x);
    build.y = static_cast<std::int16_t>(
        origin_y + kWorldSceneEffectHeadingOffsets[heading][1] + extra_y);
    build.flags = 0x4000U;
    build.source = source;
    return build;
}

} // namespace

WorldSceneEffectSpriteBuild advance_world_scene_effect_timer_a(
    WorldFrameServiceState& state) {
    tick_world_scene_effect_timer(state.words[3], state.words[0x0B], 0x14, 9U);
    const auto frame = static_cast<std::size_t>(state.words[0x0B]);
    return build_world_scene_effect_sprite(
        state, 0, 0,
        frame < kWorldSceneEffectTimerALists.size()
            ? kWorldSceneEffectTimerALists[frame]
            : 0U);
}

void advance_world_scene_effect_frame(WorldFrameServiceState& state) {
    tick_world_scene_effect_timer(state.words[1], state.words[0x0C], 0x0A,
                                  8U);
}

WorldSceneEffectSpriteBuild advance_world_scene_effect_timer_b(
    WorldFrameServiceState& state) {
    tick_world_scene_effect_timer(
        state.words[0x0D], state.words[0x0E], 0x0A, 10U);
    const auto frame = static_cast<std::size_t>(state.words[0x0E]);
    return build_world_scene_effect_sprite(
        state, 0, -0x18,
        frame < kWorldSceneEffectTimerBLists.size()
            ? kWorldSceneEffectTimerBLists[frame]
            : 0U);
}

WorldSceneEffectSpriteBuild advance_world_scene_effect_timer_c(
    WorldFrameServiceState& state) {
    tick_world_scene_effect_timer(
        state.words[0x10], state.words[0x0F], 0x04, 0x28U);
    const auto frame = static_cast<std::size_t>(state.words[0x0F]);
    return build_world_scene_effect_sprite(
        state, 0x12, 0x30,
        frame < kWorldSceneEffectTimerCLists.size()
            ? kWorldSceneEffectTimerCLists[frame]
            : 0U);
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
