#include "shadowrun_ghidra/title_intro/title_sweep_animation_conversion.hpp"

#include <array>

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::array<std::uint32_t, 6> kTitleSweepSpriteData{
    0x0001D480U, 0x0001D494U, 0x0001D4A8U,
    0x0001D4BCU, 0x0001D4D0U, 0x0001D47CU};

// Each ROM record at 0x1C6D4 is four bytes: source-table index, signed X
// offset, and a word whose low byte becomes the next delay.
constexpr std::array<std::uint8_t, 32> kTitleSweepSourceIndices{
    0U, 5U, 0U, 1U, 0U, 1U, 0U, 1U,
    2U, 2U, 2U, 2U, 2U, 3U, 3U, 3U,
    3U, 3U, 3U, 3U, 3U, 3U, 3U, 3U,
    4U, 5U, 4U, 5U, 4U, 5U, 4U, 5U};
constexpr std::array<std::int16_t, 32> kTitleSweepHorizontalOffsets{
    0, 0, 0, 0, 0, 0, 0, 0,
    -6, -2, 2, 6, 8, -10, -8, -6,
    -4, -2, 0, 2, 4, 6, 8, 10,
    0, 0, 0, 0, 0, 0, 0, 0};
constexpr std::array<std::uint8_t, 32> kTitleSweepNextDelays{
    5U, 20U, 1U, 1U, 1U, 1U, 1U, 1U,
    0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
    0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
    1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U};

} // namespace

std::optional<TitleSweepFrame> resolve_title_sweep_frame(
    const std::uint16_t phase) {
    if (phase >= kTitleSweepSourceIndices.size()) {
        return std::nullopt;
    }
    const auto source_index = kTitleSweepSourceIndices[phase];
    return TitleSweepFrame{
        0x0001C6BCU,
        kTitleSweepHorizontalOffsets[phase],
        kTitleSweepNextDelays[phase],
        kTitleSweepSpriteData[source_index]};
}

TitleSweepAnimationAction advance_title_sweep_animation(
    TitleSweepAnimationState& state,
    const TitleSweepAnimationCallbacks& callbacks) {
    state.frame_delay = static_cast<std::int8_t>(state.frame_delay - 1);
    if (state.frame_delay >= 0) {
        return TitleSweepAnimationAction::waiting;
    }

    ++state.phase;
    if (state.phase >= 0x20U) {
        // At phase 0x20 the ROM has already read the low byte at
        // 0x1C754+2 (the first instruction after the table), which is 0xFF,
        // before taking the terminal branch. Preserve that observable byte
        // state without exposing the code bytes as runtime data.
        state.frame_delay = static_cast<std::int8_t>(0xFFU);
        state.record_flags = static_cast<std::uint8_t>(
            state.record_flags & ~0x09U);
        if (callbacks.attach_behavior_descriptor_b) {
            callbacks.attach_behavior_descriptor_b();
        }
        state.transition_state = 0U;
        if (callbacks.return_to_title_state) {
            callbacks.return_to_title_state();
        }
        return TitleSweepAnimationAction::finalized;
    }

    const auto frame = callbacks.resolve_frame
                           ? callbacks.resolve_frame(state.phase)
                           : resolve_title_sweep_frame(state.phase);
    if (!frame.has_value()) {
        return TitleSweepAnimationAction::frame_unresolved;
    }
    state.frame_delay = static_cast<std::int8_t>(frame->next_delay);
    if ((state.record_flags & 0x08U) != 0U) {
        return TitleSweepAnimationAction::waiting;
    }
    if (callbacks.build_sprite) {
        callbacks.build_sprite(TitleSweepSpriteBuild{
            frame->source_table_token, state.phase,
            static_cast<std::uint16_t>(state.base_x + frame->horizontal_offset),
            state.base_y, 0x6000U, frame->sprite_data_token});
        callbacks.build_sprite(TitleSweepSpriteBuild{
            frame->source_table_token, state.phase,
            static_cast<std::uint16_t>(state.base_x - frame->horizontal_offset),
            state.base_y, 0x6000U, frame->sprite_data_token});
    }
    return TitleSweepAnimationAction::rendered;
}

} // namespace shadowrun::ghidra_conversion
