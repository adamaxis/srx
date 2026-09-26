#include "shadowrun_ghidra/rendering/display_transition_conversion.hpp"

#include <vector>

namespace {

int expect(const bool condition) {
    return condition ? 0 : 1;
}

} // namespace

int main() {
    using namespace shadowrun::ghidra_conversion;

    DisplayTransitionState state{};
    std::uint16_t audio_command = 0;
    int uploads = 0;
    int resets = 0;
    int copies = 0;
    int ticks = 0;
    std::vector<DisplayTransitionTransferRequest> transfers;
    const DisplayTransitionCallbacks callbacks{
        [&](const std::uint8_t command) { audio_command = command; },
        [&] { ++uploads; },
        [&] { ++resets; },
        [&] { ++copies; },
        [&](const DisplayTransitionTransferRequest& request) {
            transfers.push_back(request);
        },
        [&] { ++ticks; },
        [&] { ++ticks; },
        [&] { ++ticks; }};

    run_display_transition(state, callbacks);
    if (expect(state.audio_marker < 0 && audio_command == 0x29U &&
               uploads == 1 && resets == 1 && copies == 1 &&
               state.display_transfer_dirty && state.completed &&
               state.phase == DisplayTransitionPhase::final_wait &&
               transfers.size() == 3U + 14U * 4U + 6U * 2U + 4U &&
               transfers.front().word_count == 40U &&
               transfers[1].style ==
                   DisplayTransitionTransferStyle::masked_with_display_control &&
               transfers[3].phase == DisplayTransitionPhase::sweep &&
               transfers[3].iteration == 0U &&
               transfers.back().phase == DisplayTransitionPhase::clear_three &&
               ticks == (14 + 6 + 11 + 6 + 6 + 11) * 3) != 0) {
        return 1;
    }
    return 0;
}
