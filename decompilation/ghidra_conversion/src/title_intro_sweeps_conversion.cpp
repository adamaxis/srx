#include "shadowrun_ghidra/title_intro_sweeps_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

void reset_linear_sweep(TitleIntroLinearSweepState& state) {
    state.next_event_frame = 0;
    state.phase = 0;
}

void advance_linear_sweep(
    TitleIntroLinearSweepState& state,
    const std::array<TitleIntroLinearSweepPhase,
                     kTitleIntroLinearSweepPhaseCount>& phases,
    const std::uint16_t decrement,
    const TitleIntroSweepDrawBoundary& draw_sprite) {
    if (state.current_frame >= state.next_event_frame) {
        if (state.phase >= kTitleIntroLinearSweepPhaseCount) {
            return;
        }
        const auto& phase = phases[state.phase];
        state.sprite_descriptor = phase.sprite_descriptor;
        state.base_y = phase.base_y;
        state.next_event_frame = static_cast<std::uint16_t>(
            state.current_frame + phase.delay_frames);
        state.current_x = 0x0200U;
        ++state.phase;
    } else if (state.current_x < 0x0040U) {
        return;
    }

    state.current_x = static_cast<std::uint16_t>(state.current_x - decrement);
    if (draw_sprite) {
        draw_sprite({state.sprite_descriptor, state.current_x,
                     static_cast<std::uint16_t>(state.base_y + 0x0018U),
                     0x6000U});
    }
}

} // namespace

void reset_title_intro_targeted_sweep(TitleIntroTargetedSweepState& state) {
    state.next_event_frame = 0;
    state.phase = 0;
}

void advance_title_intro_targeted_sweep(
    TitleIntroTargetedSweepState& state,
    const std::array<TitleIntroTargetedSweepPhase,
                     kTitleIntroTargetedSweepPhaseCount>& phases,
    const TitleIntroSweepDrawBoundary& draw_sprite) {
    if (state.current_frame < state.next_event_frame) {
        if (state.current_x < state.target_x) {
            return;
        }
    } else {
        if (state.phase >= kTitleIntroTargetedSweepPhaseCount) {
            return;
        }
        const auto& phase = phases[state.phase];
        state.sprite_descriptor = phase.sprite_descriptor;
        state.base_y = phase.base_y;
        state.target_x = phase.target_x;
        state.next_event_frame = static_cast<std::uint16_t>(
            state.current_frame + phase.delay_frames);
        state.current_x = 0x0200U;
        ++state.phase;
    }

    state.current_x = static_cast<std::uint16_t>(state.current_x - 9U);
    if (draw_sprite) {
        draw_sprite({state.sprite_descriptor, state.current_x,
                     static_cast<std::uint16_t>(state.base_y + 0x0018U),
                     0x6000U});
    }
}

void reset_title_intro_linear_sweep_a(TitleIntroLinearSweepState& state) {
    reset_linear_sweep(state);
}

void advance_title_intro_linear_sweep_a(
    TitleIntroLinearSweepState& state,
    const std::array<TitleIntroLinearSweepPhase,
                     kTitleIntroLinearSweepPhaseCount>& phases,
    const TitleIntroSweepDrawBoundary& draw_sprite) {
    advance_linear_sweep(state, phases, 8U, draw_sprite);
}

void reset_title_intro_linear_sweep_b(TitleIntroLinearSweepState& state) {
    reset_linear_sweep(state);
}

void advance_title_intro_linear_sweep_b(
    TitleIntroLinearSweepState& state,
    const std::array<TitleIntroLinearSweepPhase,
                     kTitleIntroLinearSweepPhaseCount>& phases,
    const TitleIntroSweepDrawBoundary& draw_sprite) {
    advance_linear_sweep(state, phases, 0x000BU, draw_sprite);
}

} // namespace shadowrun::ghidra_conversion
