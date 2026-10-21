#include "shadowrun_ghidra/session/save_field_restore_conversion.hpp"

#include <algorithm>

namespace shadowrun::ghidra_conversion {
namespace {

constexpr std::size_t kPrimaryMarkerOffset = 0x212U;
constexpr std::size_t kSecondaryMarkerOffset = 0x213U;

std::uint16_t read_big_endian_word(const std::vector<std::uint8_t>& bytes,
                                   const std::size_t offset) {
    return static_cast<std::uint16_t>(
        (static_cast<std::uint16_t>(bytes[offset]) << 8U) |
        bytes[offset + 1U]);
}

bool marker_is_active(const std::uint8_t marker) {
    return static_cast<std::int8_t>(marker) >= 0;
}

std::uint16_t marker_to_template_index(const std::uint8_t marker) {
    return static_cast<std::uint16_t>(marker - 1U);
}

} // namespace

SaveFieldRestoreResult restore_saved_field_state(
    const std::vector<std::uint8_t>& source_work_buffer,
    const std::vector<RunnerRosterTemplate>& roster_templates,
    std::vector<CompactRunnerRecord>& compact_records,
    const bool world_effect_pending,
    const std::uint8_t scene_profile_selector,
    SaveFieldRestoreState& state,
    const SaveFieldRestoreCallbacks& callbacks) {
    SaveFieldRestoreResult result{};
    if (source_work_buffer.size() < kSaveFieldImageSize) {
        return result;
    }

    const auto source_begin =
        source_work_buffer.begin() + static_cast<std::ptrdiff_t>(
                                         kSaveFieldSourceOffset);
    std::copy_n(source_begin, kSaveFieldPayloadSize,
                state.work_payload.begin());
    state.dialogue_flags =
        state.work_payload[kSaveFieldDialogueFlagsOffset];

    // The saved markers are inside the copied payload at the offsets that
    // became FFFC0C and FFFC0D after the source copy.
    state.roster.primary_marker = state.work_payload[kPrimaryMarkerOffset];
    state.roster.secondary_marker = state.work_payload[kSecondaryMarkerOffset];
    const auto tail = kSaveFieldSourceOffset + kSaveFieldPayloadSize;
    state.world_mode = read_big_endian_word(source_work_buffer, tail);

    if (callbacks.prepare_field_scene_buffers) {
        callbacks.prepare_field_scene_buffers(state);
    }
    state.field_scene_prepared = true;
    state.field_flags = 0x0FU;

    // 0x59518..0x59542 loads each marker, rejects the signed 0xFF sentinel,
    // subtracts one, clears D7, and calls the corresponding template loader.
    if (marker_is_active(state.roster.primary_marker)) {
        const auto loaded = initialize_primary_roster_template(
            marker_to_template_index(state.roster.primary_marker),
            roster_templates, 0U, state.roster, compact_records);
        result.primary_template_loaded =
            loaded.action == RunnerRosterTemplateAction::loaded;
    }
    if (marker_is_active(state.roster.secondary_marker)) {
        const auto loaded = initialize_secondary_roster_template(
            marker_to_template_index(state.roster.secondary_marker),
            roster_templates, 0U, state.roster, compact_records);
        result.secondary_template_loaded =
            loaded.action == RunnerRosterTemplateAction::loaded;
        result.secondary_template_skipped_as_duplicate =
            loaded.action == RunnerRosterTemplateAction::skipped_duplicate_primary;
    }

    state.camera_x = read_big_endian_word(source_work_buffer, tail + 2U);
    state.camera_y = read_big_endian_word(source_work_buffer, tail + 4U);
    for (std::size_t index = 0; index < state.runner_positions.size(); ++index) {
        const auto position_offset = tail + 6U + index * 4U;
        state.runner_positions[index].x =
            read_big_endian_word(source_work_buffer, position_offset);
        state.runner_positions[index].y =
            read_big_endian_word(source_work_buffer, position_offset + 2U);
    }

    result.population =
        refresh_runner_record_population(state.roster, compact_records);
    state.recruited_runner_effect_flags[0] =
        marker_is_active(state.roster.primary_marker) ? 0x0DU : 0U;
    state.recruited_runner_effect_flags[1] =
        marker_is_active(state.roster.secondary_marker) ? 0x0DU : 0U;
    state.active_runner_slot = 0U;

    // The source starts with 0x720FA, retains it only for selector 2, uses
    // 0x7223A for selector 3, and otherwise selects 0x7219A.
    state.field_profile = scene_profile_selector == 2U
                              ? FieldProfileSelection::profile_0x720fa
                              : scene_profile_selector == 3U
                                    ? FieldProfileSelection::profile_0x7223a
                                    : FieldProfileSelection::profile_0x7219a;

    state.world_effect_requested = world_effect_pending;
    if (world_effect_pending &&
        callbacks.spawn_world_effect_at_player_position) {
        callbacks.spawn_world_effect_at_player_position(state);
    }

    result.action = SaveFieldRestoreAction::restored;
    return result;
}

} // namespace shadowrun::ghidra_conversion
