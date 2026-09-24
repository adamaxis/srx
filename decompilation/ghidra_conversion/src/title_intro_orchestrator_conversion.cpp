#include "shadowrun_ghidra/title_intro_orchestrator_conversion.hpp"

#include "shadowrun_ghidra/title_intro_stage_conversion.hpp"

namespace shadowrun::ghidra_conversion {

TitleIntroOrchestratorStopReason run_title_intro_orchestrator(
    TitleIntroOrchestratorState& state,
    const TitleIntroOrchestratorAssets& assets,
    const std::array<TitleIntroStageTimedPhase,
                     kTitleIntroStageTimedPhaseCount>& timed_phases,
    const std::uint32_t base_sprite_descriptor,
    const std::uint32_t projection_sprite_descriptor,
    const TitleIntroOrchestratorCallbacks& callbacks) {
    if (callbacks.reset_upload_state) {
        callbacks.reset_upload_state();
    }
    clear_title_intro_display_buffer(callbacks.clear_display_buffer);
    if (callbacks.clear_plane_a) {
        callbacks.clear_plane_a();
    }
    if (callbacks.clear_plane_b) {
        callbacks.clear_plane_b();
    }
    if (callbacks.decode_and_upload) {
        callbacks.decode_and_upload({assets.first_graphics_asset, 0x3020U});
        callbacks.decode_and_upload({assets.second_graphics_asset, 0xC000U});
    }
    if (callbacks.load_title_font) {
        callbacks.load_title_font();
    }
    render_title_intro_stage_text_batch(assets.text_assets,
                                        callbacks.render_text);
    if (callbacks.wait_for_vblank) {
        callbacks.wait_for_vblank();
    }
    if (callbacks.copy_target_palette) {
        callbacks.copy_target_palette(assets.palette_asset);
    }
    fill_title_intro_word_block(
        {0x0786U, 20U, 13U, 0x0080U, 0x07FFU}, callbacks.write_fill_row);

    state.animation_start_frame = state.current_frame;
    state.timed_state.current_frame = state.current_frame;
    state.timed_state.animation_start_frame = state.animation_start_frame;
    state.frame_state = {};
    reset_title_intro_stage_projection(state.projection_state);
    reset_title_intro_stage_timed_sprites(state.timed_state);
    service_title_intro_stage_frame(
        state.frame_state, state.timed_state, timed_phases,
        base_sprite_descriptor, state.projection_state,
        projection_sprite_descriptor, callbacks.draw_sprite);

    if (callbacks.wait_for_vblank) {
        callbacks.wait_for_vblank();
    }
    if (callbacks.render_hud_sprites) {
        callbacks.render_hud_sprites(
            {assets.hud_sprite_asset, 0x0700U, 0x0028U, 0x000EU, 0x8000U,
             0U});
    }
    const auto frame_before_fade = state.current_frame;
    if (callbacks.advance_palette_fade) {
        callbacks.advance_palette_fade();
    }
    state.current_frame = frame_before_fade;
    if (callbacks.sample_controller) {
        callbacks.sample_controller();
    }

    for (state.loop_frame = 0; state.loop_frame <= 0x0114U;
         ++state.loop_frame) {
        if (callbacks.wait_for_vblank) {
            callbacks.wait_for_vblank();
        }
        if (callbacks.sample_controller) {
            callbacks.sample_controller();
        }
        if (callbacks.latch_controller_events) {
            callbacks.latch_controller_events();
        }
        state.timed_state.current_frame = state.current_frame;
        service_title_intro_stage_frame(
            state.frame_state, state.timed_state, timed_phases,
            base_sprite_descriptor, state.projection_state,
            projection_sprite_descriptor, callbacks.draw_sprite);
        advance_title_intro_stage_service_steps(
            state.current_frame, state.animation_start_frame,
            callbacks.service_step);
        if (callbacks.stop_requested && callbacks.stop_requested()) {
            return TitleIntroOrchestratorStopReason::external_stop;
        }
    }
    return TitleIntroOrchestratorStopReason::source_complete;
}

} // namespace shadowrun::ghidra_conversion
