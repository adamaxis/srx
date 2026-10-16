#pragma once

#include "shadowrun_ghidra/session/runner_roster_conversion.hpp"

#include <array>
#include <cstdint>
#include <functional>
#include <vector>

namespace shadowrun::ghidra_conversion {

constexpr std::size_t kSaveFieldSourceOffset = 0x3D6U;
constexpr std::size_t kSaveFieldPayloadSize = 0x248U;
constexpr std::size_t kSaveFieldTailSize = 0x12U;
constexpr std::size_t kSaveFieldImageSize =
    kSaveFieldSourceOffset + kSaveFieldPayloadSize + kSaveFieldTailSize;

struct RestoredRunnerPosition {
    std::uint16_t x = 0;
    std::uint16_t y = 0;
};

enum class FieldProfileSelection {
    profile_0x720fa,
    profile_0x7219a,
    profile_0x7223a,
};

struct SaveFieldRestoreState {
    // Host-owned copy of the source destination at FFF9FA. Unknown save
    // fields remain opaque until another consumer proves their meaning.
    std::array<std::uint8_t, kSaveFieldPayloadSize> work_payload{};
    std::uint16_t world_mode = 0; // source FFDDC4
    std::uint16_t camera_x = 0;   // source FFDDC6
    std::uint16_t camera_y = 0;   // source FFDDC8
    std::array<RestoredRunnerPosition, 3> runner_positions{};

    RunnerRosterState roster{};
    std::uint8_t field_flags = 0;
    std::array<std::uint8_t, 2> recruited_runner_effect_flags{};
    std::size_t active_runner_slot = 0;
    FieldProfileSelection field_profile = FieldProfileSelection::profile_0x720fa;
    bool field_scene_prepared = false;
    bool world_effect_requested = false;
};

struct SaveFieldRestoreCallbacks {
    // The direct callees at 0x03074 and 0x1E4AC remain separate conversion
    // boundaries and receive the typed restore state rather than RAM globals.
    std::function<void(SaveFieldRestoreState&)> prepare_field_scene_buffers;
    std::function<void(SaveFieldRestoreState&)>
        spawn_world_effect_at_player_position;
};

enum class SaveFieldRestoreAction {
    restored,
    source_buffer_too_small,
};

struct SaveFieldRestoreResult {
    SaveFieldRestoreAction action = SaveFieldRestoreAction::source_buffer_too_small;
    bool primary_template_loaded = false;
    bool secondary_template_loaded = false;
    bool secondary_template_skipped_as_duplicate = false;
    RunnerRecordPopulationResult population{};
};

// ROM 0x594E0: restore the saved field payload, repopulate Joshua plus the
// selected runner records, restore the field coordinates, choose the active
// field profile, and request the pending player-position effect. The source
// call order is preserved, but its RAM addresses and register plumbing are
// represented by typed host state.
SaveFieldRestoreResult restore_saved_field_state(
    const std::vector<std::uint8_t>& source_work_buffer,
    const std::vector<RunnerRosterTemplate>& roster_templates,
    std::vector<CompactRunnerRecord>& compact_records,
    bool world_effect_pending,
    std::uint8_t scene_profile_selector,
    SaveFieldRestoreState& state,
    const SaveFieldRestoreCallbacks& callbacks = {});

} // namespace shadowrun::ghidra_conversion
