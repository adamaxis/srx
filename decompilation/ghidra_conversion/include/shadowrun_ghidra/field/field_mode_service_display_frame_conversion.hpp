#pragma once

#include <array>
#include <cstdint>
#include <optional>
#include <vector>

namespace shadowrun::ghidra_conversion {

enum class FieldModeDisplayFrameTarget {
    status_zero,
    status_44_through_49,
    display_mode_one,
    status_six_through_nine,
    status_four,
    status_five,
    display_mode_two,
};

struct FieldModeDisplayFrameWrite {
    FieldModeDisplayFrameTarget target =
        FieldModeDisplayFrameTarget::status_zero;
    std::vector<std::uint16_t> words;
};

struct FieldModeDisplayFrameTables {
    // Extracted word tables corresponding to ROM 0x18AD42, 0x18AD5A,
    // 0x18AD6A, 0x18AD7A, 0x18AD92, and 0x18AD4E. Some source animations
    // deliberately use overlapping windows, so their storage lengths are
    // not simply frame_count * words_per_frame.
    std::array<std::uint16_t, 6> status_zero{};
    std::array<std::uint16_t, 7> status_44_through_49{};
    std::array<std::uint16_t, 7> display_mode_one{};
    std::array<std::uint16_t, 12> status_six_through_nine{};
    std::array<std::uint16_t, 12> status_four{};
    std::array<std::uint16_t, 5> status_five{};
};

struct FieldModeDisplayFrameState {
    bool service_suppressed = false; // FFE18A != 0
    std::uint16_t display_status = 0; // FFDDC4
    std::uint16_t display_mode = 0; // FFDF40
    std::uint16_t frame_accumulator = 0; // FFDF88
    std::int16_t frame_delay = 0; // FFDF90
    std::uint16_t frame_index = 0; // FFDF92
};

enum class FieldModeDisplayFrameAction {
    no_change,
    status_zero_frame,
    status_44_through_49_frame,
    display_mode_one_frame,
    status_six_through_nine_frame,
    status_four_frame,
    status_five_frame,
    display_mode_two_frame,
};

struct FieldModeDisplayFrameResult {
    FieldModeDisplayFrameAction action =
        FieldModeDisplayFrameAction::no_change;
    std::optional<FieldModeDisplayFrameWrite> write;
};

// ROM 0x5672C: advance the field display's status/mode-specific cadence and
// return a typed presentation write when a frame boundary expires. VDP
// control/data traffic is represented as a named host target plus words; the
// conversion does not emulate the VDP or retain Genesis address state.
FieldModeDisplayFrameResult service_field_mode_display_frame(
    FieldModeDisplayFrameState& state,
    const FieldModeDisplayFrameTables& tables);

} // namespace shadowrun::ghidra_conversion
