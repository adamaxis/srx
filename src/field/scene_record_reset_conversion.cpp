#include "shadowrun_ghidra/field/scene_record_reset_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void snapshot_scene_state(
    SceneStateSnapshot& snapshot,
    const std::array<std::uint16_t, 0x31>& source,
    const SceneStateSnapshotCallbacks& callbacks) {
    snapshot.words = source;
    if (callbacks.clear_entity_effect_pending_records) {
        callbacks.clear_entity_effect_pending_records();
    }
    if (callbacks.clear_effect_slots) {
        callbacks.clear_effect_slots();
    }
}

void clear_scene_record_transition_words(SceneRecordResetState& state) {
    state.transition_words.fill(0U);
}

void clear_scene_record_pending_flags(SceneRecordResetState& state) {
    for (auto& flags : state.pending_flags) {
        flags = static_cast<std::uint8_t>(flags & 0xFEU);
    }
}

bool all_scene_record_slots_active(
    const std::array<std::uint8_t, 10>& active_flags) {
    for (const auto flags : active_flags) {
        if ((flags & 0x01U) == 0U) {
            return false;
        }
    }
    return true;
}

} // namespace shadowrun::ghidra_conversion
