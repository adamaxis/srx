#include "shadowrun_ghidra/field/field_mode_service_display_frame_conversion.hpp"

#include <cstdint>

int main() {
    using namespace shadowrun::ghidra_conversion;

    FieldModeDisplayFrameTables tables{};
    for (std::size_t index = 0; index < tables.status_zero.size(); ++index) {
        tables.status_zero[index] = static_cast<std::uint16_t>(0x100U + index);
    }
    for (std::size_t index = 0; index < tables.status_44_through_49.size();
         ++index) {
        tables.status_44_through_49[index] =
            static_cast<std::uint16_t>(0x200U + index);
        tables.display_mode_one[index] =
            static_cast<std::uint16_t>(0x300U + index);
    }
    for (std::size_t index = 0; index < tables.status_six_through_nine.size();
         ++index) {
        tables.status_six_through_nine[index] =
            static_cast<std::uint16_t>(0x400U + index);
        tables.status_four[index] = static_cast<std::uint16_t>(0x500U + index);
    }
    for (std::size_t index = 0; index < tables.status_five.size(); ++index) {
        tables.status_five[index] = static_cast<std::uint16_t>(0x600U + index);
    }

    FieldModeDisplayFrameState state{};
    state.display_status = 0U;
    state.frame_delay = 0;
    state.frame_index = 5U;
    auto zero = service_field_mode_display_frame(state, tables);
    if (zero.action != FieldModeDisplayFrameAction::status_zero_frame ||
        !zero.write.has_value() || state.frame_delay != 6 ||
        state.frame_index != 0U || zero.write->words !=
            std::vector<std::uint16_t>{0x100U}) {
        return 1;
    }
    const auto waiting = service_field_mode_display_frame(state, tables);
    if (waiting.write.has_value() || state.frame_delay != 5) {
        return 2;
    }

    state.display_status = 0x2CU;
    state.frame_delay = 0;
    state.frame_index = 3U;
    const auto grouped = service_field_mode_display_frame(state, tables);
    if (grouped.action !=
            FieldModeDisplayFrameAction::status_44_through_49_frame ||
        !grouped.write.has_value() || state.frame_index != 0U ||
        grouped.write->words !=
            std::vector<std::uint16_t>{0x200U, 0x201U, 0x202U, 0x203U}) {
        return 3;
    }

    state.display_mode = 1U;
    state.display_status = 0x14U;
    state.frame_delay = 0;
    state.frame_index = 3U;
    const auto mode_one_blocked =
        service_field_mode_display_frame(state, tables);
    if (mode_one_blocked.write.has_value() || state.frame_index != 3U) {
        return 4;
    }

    state.display_status = 0U;
    state.frame_delay = 0;
    state.frame_index = 3U;
    const auto mode_one = service_field_mode_display_frame(state, tables);
    if (mode_one.action != FieldModeDisplayFrameAction::display_mode_one_frame ||
        !mode_one.write.has_value() || state.frame_index != 0U ||
        mode_one.write->words != std::vector<std::uint16_t>{
            0x300U, 0x301U, 0x302U, 0x303U}) {
        return 5;
    }

    state.display_mode = 2U;
    state.frame_accumulator = 0x0FU;
    state.frame_delay = 0;
    state.frame_index = 7U;
    const auto mode_two = service_field_mode_display_frame(state, tables);
    if (mode_two.action != FieldModeDisplayFrameAction::display_mode_two_frame ||
        !mode_two.write.has_value() || state.frame_delay != -1 ||
        state.frame_index != 7U || mode_two.write->words !=
            std::vector<std::uint16_t>{static_cast<std::uint16_t>(7U << 9U)}) {
        return 6;
    }

    state.display_mode = 0U;
    state.display_status = 5U;
    state.frame_delay = 0;
    state.frame_index = 2U;
    const auto status_five = service_field_mode_display_frame(state, tables);
    if (status_five.action != FieldModeDisplayFrameAction::status_five_frame ||
        !status_five.write.has_value() || status_five.write->words !=
            std::vector<std::uint16_t>{0x600U, 0x601U, 0x602U}) {
        return 7;
    }

    state.display_status = 6U;
    state.frame_delay = 0;
    state.frame_index = 2U;
    const auto status_six = service_field_mode_display_frame(state, tables);
    if (status_six.action !=
            FieldModeDisplayFrameAction::status_six_through_nine_frame ||
        !status_six.write.has_value() || status_six.write->words !=
            std::vector<std::uint16_t>{0x400U, 0x401U, 0x402U}) {
        return 8;
    }

    state.display_status = 0x32U;
    state.frame_delay = 0;
    const auto unsupported = service_field_mode_display_frame(state, tables);
    if (unsupported.action != FieldModeDisplayFrameAction::no_change ||
        unsupported.write.has_value()) {
        return 9;
    }
    return 0;
}
