#include "shadowrun_ghidra/title_intro_persistent_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroPersistentSequenceState state{8U, 12U, 3U, 0xAAAAU, 9U};
    reset_title_intro_persistent_sequence(state);
    if (state.next_event_frame != 0U || state.stage != 0U ||
        state.current_sprite_descriptor != 0xAAAAU ||
        state.related_phase_marker != 9U) {
        return 1;
    }

    std::array<TitleIntroPersistentPhase, kTitleIntroPersistentPhaseCount>
        phases{{{0x2000U, 2U}, {0x2001U, 3U}, {0x2002U, 4U},
                {0x2003U, 5U}}};
    std::vector<std::uint16_t> audio;
    std::vector<std::uint16_t> transition_values;
    std::vector<std::uint16_t> anchors;
    std::vector<TitleIntroPersistentFinalOverlayRequest> finals;
    std::vector<TitleIntroPersistentSpriteRequest> persistent_sprites;
    int repeated_steps = 0;
    const TitleIntroPersistentCallbacks callbacks{
        [&audio](const std::uint16_t event) { audio.push_back(event); },
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
        [&finals](const TitleIntroPersistentFinalOverlayRequest& request) {
            finals.push_back(request);
        },
        [&persistent_sprites](const TitleIntroPersistentSpriteRequest& request) {
            persistent_sprites.push_back(request);
        },
    };

    state.current_frame = 0U;
    if (advance_title_intro_persistent_sequence(state, phases, callbacks) !=
            TitleIntroPersistentAdvanceResult::phase_advanced ||
        state.current_sprite_descriptor != 0x2000U || state.stage != 1U ||
        state.next_event_frame != 2U || persistent_sprites.size() != 1U ||
        persistent_sprites.front().x != 0x0178U) {
        return 2;
    }

    state.stage = 4U;
    state.current_frame = 10U;
    state.next_event_frame = 10U;
    if (advance_title_intro_persistent_sequence(state, phases, callbacks) !=
            TitleIntroPersistentAdvanceResult::repeated_stage_advanced ||
        audio != std::vector<std::uint16_t>{0x2FU} ||
        transition_values != std::vector<std::uint16_t>{1U, 0x0EEEU} ||
        state.related_phase_marker != 4U || state.stage != 5U ||
        state.next_event_frame != 19U) {
        return 3;
    }

    state.stage = 8U;
    state.current_frame = 19U;
    state.next_event_frame = 19U;
    if (advance_title_intro_persistent_sequence(state, phases, callbacks) !=
            TitleIntroPersistentAdvanceResult::repeated_stage_advanced ||
        repeated_steps != 1 ||
        anchors != std::vector<std::uint16_t>{0x0182U, 0x00E2U} ||
        state.stage != 9U) {
        return 4;
    }

    state.stage = 11U;
    state.current_frame = 28U;
    state.next_event_frame = 28U;
    if (advance_title_intro_persistent_sequence(state, phases, callbacks) !=
            TitleIntroPersistentAdvanceResult::final_overlay_advanced ||
        repeated_steps != 2 || finals.size() != 1U ||
        finals.front().destination_word != 0x03B8U || state.stage != 12U ||
        persistent_sprites.size() != 4U) {
        return 5;
    }
    if (advance_title_intro_persistent_sequence(state, phases, callbacks) !=
        TitleIntroPersistentAdvanceResult::no_op) {
        return 6;
    }
    return 0;
}
