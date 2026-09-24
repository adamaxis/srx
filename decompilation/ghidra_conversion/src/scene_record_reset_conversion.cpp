#include "shadowrun_ghidra/scene_record_reset_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void clear_scene_record_transition_words(SceneRecordResetState& state) {
    state.transition_words.fill(0U);
}

void clear_scene_record_pending_flags(SceneRecordResetState& state) {
    for (auto& flags : state.pending_flags) {
        flags = static_cast<std::uint8_t>(flags & 0xFEU);
    }
}

} // namespace shadowrun::ghidra_conversion
