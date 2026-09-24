#include "shadowrun_ghidra/scene_record_reset_conversion.hpp"

#include <array>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    SceneRecordResetState state{};
    state.transition_words.fill(0x1234U);
    state.pending_flags.fill(0xFFU);

    SceneStateSnapshot snapshot{};
    std::array<std::uint16_t, 0x31> source{};
    source.front() = 0x1234U;
    source.back() = 0x5678U;
    std::vector<int> snapshot_order;
    snapshot_scene_state(
        snapshot, source,
        {[&snapshot_order] { snapshot_order.push_back(1); },
         [&snapshot_order] { snapshot_order.push_back(2); }});
    if (snapshot.words != source ||
        snapshot_order != std::vector<int>{1, 2}) {
        return 1;
    }

    clear_scene_record_transition_words(state);
    for (const auto word : state.transition_words) {
        if (word != 0U) {
            return 2;
        }
    }

    clear_scene_record_pending_flags(state);
    for (const auto flags : state.pending_flags) {
        if (flags != 0xFEU) {
            return 3;
        }
    }

    std::array<std::uint8_t, 10> active_flags{};
    active_flags.fill(1U);
    if (!all_scene_record_slots_active(active_flags)) {
        return 4;
    }
    active_flags[6] = 0U;
    if (all_scene_record_slots_active(active_flags)) {
        return 5;
    }
    return 0;
}
