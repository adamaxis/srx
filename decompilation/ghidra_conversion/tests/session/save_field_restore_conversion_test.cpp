#include "shadowrun_ghidra/session/save_field_restore_conversion.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace {

bool expect(const bool condition) {
    return condition;
}

void write_big_endian_word(std::vector<std::uint8_t>& bytes,
                           const std::size_t offset,
                           const std::uint16_t value) {
    bytes[offset] = static_cast<std::uint8_t>(value >> 8U);
    bytes[offset + 1U] = static_cast<std::uint8_t>(value & 0xFFU);
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    std::vector<std::uint8_t> source(kSaveFieldImageSize, 0U);
    const auto payload = kSaveFieldSourceOffset;
    source[payload + 0x212U] = 2U;
    source[payload + 0x213U] = 3U;
    write_big_endian_word(source, payload + 0x248U, 0x0014U);
    write_big_endian_word(source, payload + 0x24AU, 0x1234U);
    write_big_endian_word(source, payload + 0x24CU, 0x5678U);
    write_big_endian_word(source, payload + 0x24EU, 0x0102U);
    write_big_endian_word(source, payload + 0x250U, 0x0304U);
    write_big_endian_word(source, payload + 0x252U, 0x0506U);
    write_big_endian_word(source, payload + 0x254U, 0x0708U);
    write_big_endian_word(source, payload + 0x256U, 0x090AU);
    write_big_endian_word(source, payload + 0x258U, 0x0B0CU);

    std::vector<RunnerRosterTemplate> templates(4U);
    templates[1U].record.fill(0x11U);
    templates[2U].record.fill(0x22U);
    templates[3U].record.fill(0x33U);
    std::vector<CompactRunnerRecord> compact_records(4U);
    for (std::size_t index = 0; index < compact_records.size(); ++index) {
        compact_records[index].bytes[0x00U] =
            static_cast<std::uint8_t>(index + 0x40U);
    }

    SaveFieldRestoreState state{};
    std::vector<int> callback_order;
    const SaveFieldRestoreCallbacks callbacks{
        [&callback_order](SaveFieldRestoreState& observed) {
            callback_order.push_back(1);
            if (observed.scene_mode != 0x0014U ||
                observed.roster.primary_marker != 2U ||
                observed.roster.secondary_marker != 3U) {
                callback_order.push_back(99);
            }
        },
        [&callback_order](SaveFieldRestoreState& observed) {
            callback_order.push_back(2);
            observed.world_effect_requested = true;
        }};

    const auto restored = restore_saved_field_state(
        source, templates, compact_records, true, 3U, state, callbacks);
    if (!expect(restored.action == SaveFieldRestoreAction::restored &&
                restored.primary_template_loaded &&
                restored.secondary_template_loaded &&
                callback_order == std::vector<int>{1, 2} &&
                state.scene_mode == 0x0014U && state.camera_x == 0x1234U &&
                state.camera_y == 0x5678U &&
                state.runner_positions[0].x == 0x0102U &&
                state.runner_positions[0].y == 0x0304U &&
                state.runner_positions[2].x == 0x090AU &&
                state.runner_positions[2].y == 0x0B0CU &&
                state.roster.primary_marker == 2U &&
                state.roster.secondary_marker == 3U &&
                state.recruited_runner_effect_flags[0] == 0x0DU &&
                state.recruited_runner_effect_flags[1] == 0x0DU &&
                state.field_profile == FieldProfileSelection::profile_0x7223a &&
                state.work_payload[0x212U] == 2U)) {
        return 1;
    }

    source[payload + 0x213U] = 0xFFU;
    state = SaveFieldRestoreState{};
    const auto no_secondary = restore_saved_field_state(
        source, templates, compact_records, false, 2U, state, {});
    if (!expect(no_secondary.action == SaveFieldRestoreAction::restored &&
                no_secondary.primary_template_loaded &&
                !no_secondary.secondary_template_loaded &&
                state.recruited_runner_effect_flags[0] == 0x0DU &&
                state.recruited_runner_effect_flags[1] == 0U &&
                state.field_profile == FieldProfileSelection::profile_0x720fa &&
                !state.world_effect_requested)) {
        return 2;
    }

    const auto too_small = restore_saved_field_state(
        std::vector<std::uint8_t>(kSaveFieldImageSize - 1U), templates,
        compact_records, false, 0U, state, {});
    if (!expect(too_small.action == SaveFieldRestoreAction::source_buffer_too_small)) {
        return 3;
    }

    return 0;
}
