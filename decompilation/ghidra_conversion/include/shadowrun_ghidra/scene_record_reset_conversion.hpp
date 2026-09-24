#pragma once

#include <array>
#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct SceneRecordResetState {
    std::array<std::uint16_t, 10> transition_words{};
    std::array<std::uint8_t, 10> pending_flags{};
};

struct SceneStateSnapshot {
    std::array<std::uint16_t, 0x31> words{};
};

struct SceneStateSnapshotCallbacks {
    std::function<void()> clear_entity_effect_pending_records;
    std::function<void()> clear_effect_slots;
};

// ROM 0x16E30: snapshot the 0x31-word scene state before clearing the
// pending entity effects and effect-slot occupancy tables.
void snapshot_scene_state(
    SceneStateSnapshot& snapshot,
    const std::array<std::uint16_t, 0x31>& source,
    const SceneStateSnapshotCallbacks& callbacks);

// ROM 0x164DC: clear the leading transition word in ten scene records.
void clear_scene_record_transition_words(SceneRecordResetState& state);

// ROM 0x16DEE: clear bit 0 in the ten scene-record pending flags.
void clear_scene_record_pending_flags(SceneRecordResetState& state);

// ROM 0x16E08: report whether bit 0 is set for all ten scene-record slots.
bool all_scene_record_slots_active(
    const std::array<std::uint8_t, 10>& active_flags);

} // namespace shadowrun::ghidra_conversion
