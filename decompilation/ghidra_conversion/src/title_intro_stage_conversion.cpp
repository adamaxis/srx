#include "shadowrun_ghidra/title_intro_stage_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void render_title_intro_stage_text_batch(
    const std::array<std::uint32_t, kTitleIntroStageTextCount>& text_assets,
    const std::function<void(const TitleIntroStageTextRequest&)>& render_text) {
    if (!render_text) {
        return;
    }
    constexpr std::array<std::uint16_t, kTitleIntroStageTextCount>
        kDestinationWords{{0x2186U, 0x2286U, 0x2386U}};
    for (std::size_t index = 0; index < text_assets.size(); ++index) {
        render_text({text_assets[index], kDestinationWords[index], 0x8000U});
    }
}

void reset_title_intro_stage_projection(TitleIntroStageProjectionState& state) {
    state.x_accumulator = 0;
    state.y_accumulator = 0;
}

void draw_title_intro_stage_base_sprite(
    const std::uint32_t sprite_descriptor,
    const std::function<void(const TitleIntroStageSpriteRequest&)>& draw_sprite) {
    if (draw_sprite) {
        draw_sprite({sprite_descriptor, 0x00C2U, 0x013CU, 0xC000U});
    }
}

void advance_title_intro_stage_projection(
    TitleIntroStageProjectionState& state,
    const std::uint32_t sprite_descriptor,
    const std::function<void(const TitleIntroStageSpriteRequest&)>& draw_sprite) {
    if (draw_sprite) {
        draw_sprite({sprite_descriptor,
                     static_cast<std::uint16_t>(0x0060U +
                                                (state.x_accumulator >> 8U)),
                     static_cast<std::uint16_t>(0x0123U +
                                                (state.y_accumulator >> 8U)),
                     0x6000U});
    }
    state.x_accumulator = static_cast<std::uint16_t>(
        state.x_accumulator + 0x0040U);
    state.y_accumulator = static_cast<std::uint16_t>(
        state.y_accumulator + 0x0018U);
}

void reset_title_intro_stage_timed_sprites(TitleIntroStageTimedState& state) {
    state.next_event_frame = 0;
    state.phase = 0;
}

void advance_title_intro_stage_timed_sprites(
    TitleIntroStageTimedState& state,
    const std::array<TitleIntroStageTimedPhase,
                     kTitleIntroStageTimedPhaseCount>& phases,
    const std::function<void(const TitleIntroStageSpriteRequest&)>& draw_sprite) {
    const auto elapsed = static_cast<std::uint16_t>(
        state.current_frame - state.animation_start_frame);
    if (elapsed < 0x003CU) {
        return;
    }

    if (state.current_frame >= state.next_event_frame &&
        state.phase < kTitleIntroStageTimedPhaseCount) {
        const auto& phase = phases[state.phase];
        state.current_sprite_descriptor = phase.sprite_descriptor;
        state.x = phase.x;
        state.y = phase.y;
        state.next_event_frame = static_cast<std::uint16_t>(
            state.current_frame + phase.delay_frames);
        ++state.phase;
    }

    if (draw_sprite) {
        draw_sprite({state.current_sprite_descriptor, state.x, state.y,
                     0xE000U});
    }
}

void service_title_intro_stage_frame(
    TitleIntroStageFrameState& frame_state,
    TitleIntroStageTimedState& timed_state,
    const std::array<TitleIntroStageTimedPhase,
                     kTitleIntroStageTimedPhaseCount>& timed_phases,
    const std::uint32_t base_sprite_descriptor,
    TitleIntroStageProjectionState& projection_state,
    const std::uint32_t projection_sprite_descriptor,
    const std::function<void(const TitleIntroStageSpriteRequest&)>& draw_sprite) {
    frame_state.scene_mode = 1U;
    advance_title_intro_stage_timed_sprites(timed_state, timed_phases,
                                             draw_sprite);
    draw_title_intro_stage_base_sprite(base_sprite_descriptor, draw_sprite);
    advance_title_intro_stage_projection(projection_state,
                                         projection_sprite_descriptor,
                                         draw_sprite);
    frame_state.entity_record_pending = true;
    frame_state.dirty_flags = static_cast<std::uint8_t>(
        frame_state.dirty_flags | 0x02U);
}

bool advance_title_intro_stage_service_steps(
    const std::uint16_t current_frame,
    const std::uint16_t animation_start_frame,
    const std::function<void(std::uint16_t)>& service_step) {
    const auto elapsed = static_cast<std::uint16_t>(
        current_frame - animation_start_frame);
    if (elapsed <= 0x00B3U || (elapsed & 0x000BU) != 0U) {
        return false;
    }
    if (service_step) {
        for (std::uint16_t index = 0; index < 4U; ++index) {
            service_step(index);
        }
    }
    return true;
}

} // namespace shadowrun::ghidra_conversion
