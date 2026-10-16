#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct TitleIntroSweepSpriteRequest {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t x = 0;
    std::uint16_t y = 0;
    std::uint16_t palette_flags = 0;
};

using TitleIntroSweepDrawBoundary =
    std::function<void(const TitleIntroSweepSpriteRequest&)>;

struct TitleIntroTargetedSweepPhase {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t base_y = 0;
    std::uint16_t delay_frames = 0;
    std::uint16_t target_x = 0;
};

constexpr std::size_t kTitleIntroTargetedSweepPhaseCount = 7;

struct TitleIntroTargetedSweepState {
    std::uint16_t current_frame = 0;
    std::uint16_t next_event_frame = 0;
    std::uint16_t phase = 0;
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t current_x = 0;
    std::uint16_t base_y = 0;
    std::uint16_t target_x = 0;
};

// ROM 0x5A61A: reset the seven-entry targeted sweep deadline and phase.
void reset_title_intro_targeted_sweep(TitleIntroTargetedSweepState& state);

// ROM 0x5A628: select a seven-entry descriptor, move left from 0x200 by nine
// words per active call, and stop each entry when its target x is crossed.
void advance_title_intro_targeted_sweep(
    TitleIntroTargetedSweepState& state,
    const std::array<TitleIntroTargetedSweepPhase,
                     kTitleIntroTargetedSweepPhaseCount>& phases,
    const TitleIntroSweepDrawBoundary& draw_sprite);

struct TitleIntroLinearSweepPhase {
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t base_y = 0;
    std::uint16_t delay_frames = 0;
};

constexpr std::size_t kTitleIntroLinearSweepPhaseCount = 8;

struct TitleIntroLinearSweepState {
    std::uint16_t current_frame = 0;
    std::uint16_t next_event_frame = 0;
    std::uint16_t phase = 0;
    std::uint32_t sprite_descriptor = 0;
    std::uint16_t current_x = 0;
    std::uint16_t base_y = 0;
};

// ROM 0x5A70E/0x5A71C: reset and advance the eight-entry sweep with an
// eight-word decrement and a 0x40 lower cutoff.
void reset_title_intro_linear_sweep_a(TitleIntroLinearSweepState& state);
void advance_title_intro_linear_sweep_a(
    TitleIntroLinearSweepState& state,
    const std::array<TitleIntroLinearSweepPhase,
                     kTitleIntroLinearSweepPhaseCount>& phases,
    const TitleIntroSweepDrawBoundary& draw_sprite);

// ROM 0x5A7EA/0x5A7F8: same eight-entry schedule with an eleven-word
// decrement and the same 0x40 lower cutoff.
void reset_title_intro_linear_sweep_b(TitleIntroLinearSweepState& state);
void advance_title_intro_linear_sweep_b(
    TitleIntroLinearSweepState& state,
    const std::array<TitleIntroLinearSweepPhase,
                     kTitleIntroLinearSweepPhaseCount>& phases,
    const TitleIntroSweepDrawBoundary& draw_sprite);

} // namespace shadowrun::ghidra_conversion
