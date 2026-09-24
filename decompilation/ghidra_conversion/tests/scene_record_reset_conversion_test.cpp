#include "shadowrun_ghidra/scene_record_reset_conversion.hpp"

int main() {
    using namespace shadowrun::ghidra_conversion;

    SceneRecordResetState state{};
    state.transition_words.fill(0x1234U);
    state.pending_flags.fill(0xFFU);

    clear_scene_record_transition_words(state);
    for (const auto word : state.transition_words) {
        if (word != 0U) {
            return 1;
        }
    }

    clear_scene_record_pending_flags(state);
    for (const auto flags : state.pending_flags) {
        if (flags != 0xFEU) {
            return 2;
        }
    }
    return 0;
}
