#include "shadowrun_ghidra/title_intro_sequence_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroSequenceState state{19U, 8U, 3U};
    reset_title_intro_sequence(state);
    if (state.next_event_frame != 0U || state.stage != 0U ||
        state.current_frame != 19U) {
        return 1;
    }

    std::array<TitleIntroSequencePhase, kTitleIntroSequencePhaseCount> phases{{
        {0x1000U, 2U}, {0x1001U, 3U}, {0x1002U, 4U}, {0x1003U, 5U}}};
    std::vector<TitleIntroSpriteStreamRequest> streams;
    std::vector<std::uint16_t> audio;
    std::vector<std::uint16_t> transition_values;
    std::vector<std::uint16_t> anchors;
    std::vector<TitleIntroFinalOverlayRequest> finals;
    int repeated_steps = 0;
    const TitleIntroSequenceCallbacks callbacks{
        [&streams](const TitleIntroSpriteStreamRequest& request) {
            streams.push_back(request);
        },
        [&transition_values](const std::uint16_t first,
                             const std::uint16_t second) {
            transition_values.push_back(first);
            transition_values.push_back(second);
        },
        [&repeated_steps] { ++repeated_steps; },
        [&anchors](const std::uint16_t first, const std::uint16_t second) {
            anchors.push_back(first);
            anchors.push_back(second);
        },
        [&finals](const TitleIntroFinalOverlayRequest& request) {
            finals.push_back(request);
        },
        [&audio](const std::uint16_t event) { audio.push_back(event); },
    };

    state.current_frame = 0U;
    if (advance_title_intro_sequence(state, phases, callbacks) !=
            TitleIntroSequenceAdvanceResult::phase_rendered ||
        streams.size() != 1U || streams.front().sprite_descriptor != 0x1000U ||
        state.stage != 1U || state.next_event_frame != 2U) {
        return 2;
    }

    state.stage = 4U;
    state.current_frame = 10U;
    state.next_event_frame = 10U;
    if (advance_title_intro_sequence(state, phases, callbacks) !=
            TitleIntroSequenceAdvanceResult::transition_rendered ||
        audio != std::vector<std::uint16_t>{0x46U} ||
        transition_values != std::vector<std::uint16_t>{0U, 0x0EEEU} ||
        state.stage != 5U || state.next_event_frame != 19U) {
        return 3;
    }

    state.stage = 8U;
    state.current_frame = 19U;
    state.next_event_frame = 19U;
    if (advance_title_intro_sequence(state, phases, callbacks) !=
            TitleIntroSequenceAdvanceResult::repeated_stage_rendered ||
        repeated_steps != 1 ||
        anchors != std::vector<std::uint16_t>{0x013DU, 0x0139U} ||
        state.stage != 9U || state.next_event_frame != 28U) {
        return 4;
    }

    state.stage = 11U;
    state.current_frame = 28U;
    state.next_event_frame = 28U;
    if (advance_title_intro_sequence(state, phases, callbacks) !=
            TitleIntroSequenceAdvanceResult::final_overlay_rendered ||
        repeated_steps != 2 || finals.size() != 1U ||
        finals.front().destination_word != 0x0826U || state.stage != 12U) {
        return 5;
    }

    if (advance_title_intro_sequence(state, phases, callbacks) !=
        TitleIntroSequenceAdvanceResult::no_op) {
        return 6;
    }
    return 0;
}
