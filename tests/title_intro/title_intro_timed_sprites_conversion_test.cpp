#include "shadowrun_ghidra/title_intro/title_intro_timed_sprites_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroTimedSpriteState state{0U, 9U, 2U, 0xAAAAU, 0U, 0U};
    reset_title_intro_timed_sprites(state);
    if (state.next_event_frame != 0U || state.descriptor_index != 6U) {
        return 1;
    }
    begin_title_intro_timed_sprites(state, 0x013DU, 0x0139U);
    const std::array<std::uint32_t, kTitleIntroTimedSpriteCount> descriptors{{
        0x5000U, 0x5001U, 0x5002U, 0x5003U, 0x5004U, 0x5005U}};
    std::vector<TitleIntroPositionedSpriteRequest> requests;
    const TitleIntroTimedSpriteCallbacks callbacks{
        [&requests](const TitleIntroPositionedSpriteRequest& request) {
            requests.push_back(request);
        }};

    state.current_frame = 0U;
    if (advance_title_intro_timed_sprites(state, descriptors, callbacks) !=
            TitleIntroTimedSpriteAdvanceResult::sprite_rendered ||
        state.current_sprite_descriptor != 0x5000U ||
        state.next_event_frame != 8U || state.descriptor_index != 1U ||
        requests.front().x != 0x013DU || requests.front().y != 0x0139U) {
        return 2;
    }
    state.current_frame = 7U;
    advance_title_intro_timed_sprites(state, descriptors, callbacks);
    if (requests.size() != 2U || requests.back().sprite_descriptor != 0x5000U) {
        return 3;
    }
    state.current_frame = 8U;
    advance_title_intro_timed_sprites(state, descriptors, callbacks);
    if (state.current_sprite_descriptor != 0x5001U ||
        requests.back().sprite_descriptor != 0x5001U) {
        return 4;
    }

    state.current_frame = 40U;
    state.next_event_frame = 40U;
    state.descriptor_index = 5U;
    if (advance_title_intro_timed_sprites(state, descriptors, callbacks) !=
            TitleIntroTimedSpriteAdvanceResult::sequence_finished ||
        requests.back().sprite_descriptor != 0x5005U) {
        return 5;
    }
    state.current_frame = 48U;
    state.next_event_frame = 48U;
    if (advance_title_intro_timed_sprites(state, descriptors, callbacks) !=
        TitleIntroTimedSpriteAdvanceResult::no_op) {
        return 6;
    }
    return 0;
}
