#include "shadowrun_ghidra/field/field_mode_service_display_frame_conversion.hpp"

#include <algorithm>
#include <cstddef>
#include <utility>

namespace shadowrun::ghidra_conversion {
namespace {

template <std::size_t TableSize>
std::optional<FieldModeDisplayFrameWrite> advance_fixed_frame(
    FieldModeDisplayFrameState& state,
    const std::array<std::uint16_t, TableSize>& table,
    const std::uint16_t frame_reset,
    const std::uint16_t frame_count,
    const std::uint16_t source_stride,
    const std::uint16_t word_count,
    const FieldModeDisplayFrameTarget target) {
    state.frame_delay = static_cast<std::int16_t>(state.frame_delay - 1);
    if (state.frame_delay >= 0) {
        return std::nullopt;
    }

    state.frame_delay = static_cast<std::int16_t>(frame_reset);
    state.frame_index = static_cast<std::uint16_t>(state.frame_index + 1U);
    if (state.frame_index >= frame_count) {
        state.frame_index = 0;
    }

    const auto source_offset = static_cast<std::size_t>(
        state.frame_index * source_stride);
    std::vector<std::uint16_t> words;
    words.reserve(word_count);
    for (std::size_t index = 0; index < word_count; ++index) {
        words.push_back(table[source_offset + index]);
    }
    return FieldModeDisplayFrameWrite{target, std::move(words)};
}

FieldModeDisplayFrameResult frame_result(
    const FieldModeDisplayFrameAction action,
    std::optional<FieldModeDisplayFrameWrite> write) {
    return {action, std::move(write)};
}

} // namespace

FieldModeDisplayFrameResult service_field_mode_display_frame(
    FieldModeDisplayFrameState& state,
    const FieldModeDisplayFrameTables& tables) {
    if (state.service_suppressed || state.display_status == 0x14U) {
        return {};
    }

    if (state.display_mode == 1U) {
        return frame_result(
            FieldModeDisplayFrameAction::display_mode_one_frame,
            advance_fixed_frame(state, tables.display_mode_one, 6U, 4U, 1U,
                                 4U,
                                 FieldModeDisplayFrameTarget::display_mode_one));
    }

    if (state.display_mode == 2U) {
        if ((state.frame_accumulator & 0x000FU) != 0x000FU) {
            return {};
        }

        if (state.frame_delay < 0) {
            state.frame_index = static_cast<std::uint16_t>(
                (state.frame_index - 1U) & 0x0007U);
            if (state.frame_index == 7U) {
                state.frame_delay = 0;
                state.frame_index = 0;
            }
        } else {
            state.frame_index = static_cast<std::uint16_t>(
                (state.frame_index + 1U) & 0x0007U);
            if (state.frame_index == 0U) {
                state.frame_delay = -1;
                state.frame_index = 7U;
            }
        }
        return {FieldModeDisplayFrameAction::display_mode_two_frame,
                FieldModeDisplayFrameWrite{
                    FieldModeDisplayFrameTarget::display_mode_two,
                    {static_cast<std::uint16_t>(state.frame_index << 9U)}}};
    }

    if (state.display_mode != 0U) {
        return {};
    }

    if (state.display_status == 6U || state.display_status == 7U ||
        state.display_status == 8U || state.display_status == 9U) {
        return frame_result(
            FieldModeDisplayFrameAction::status_six_through_nine_frame,
            advance_fixed_frame(
                state, tables.status_six_through_nine, 12U, 3U, 4U, 3U,
                FieldModeDisplayFrameTarget::status_six_through_nine));
    }
    if (state.display_status == 4U) {
        return frame_result(
            FieldModeDisplayFrameAction::status_four_frame,
            advance_fixed_frame(state, tables.status_four, 10U, 3U, 4U, 3U,
                                 FieldModeDisplayFrameTarget::status_four));
    }
    if (state.display_status == 5U) {
        return frame_result(
            FieldModeDisplayFrameAction::status_five_frame,
            advance_fixed_frame(state, tables.status_five, 10U, 3U, 1U, 3U,
                                 FieldModeDisplayFrameTarget::status_five));
    }
    if (state.display_status == 0U) {
        return frame_result(
            FieldModeDisplayFrameAction::status_zero_frame,
            advance_fixed_frame(state, tables.status_zero, 6U, 6U, 1U, 1U,
                                 FieldModeDisplayFrameTarget::status_zero));
    }
    if (state.display_status > 0x2BU && state.display_status < 0x32U) {
        return frame_result(
            FieldModeDisplayFrameAction::status_44_through_49_frame,
            advance_fixed_frame(
                state, tables.status_44_through_49, 9U, 4U, 1U, 4U,
                FieldModeDisplayFrameTarget::status_44_through_49));
    }
    return {};
}

} // namespace shadowrun::ghidra_conversion
