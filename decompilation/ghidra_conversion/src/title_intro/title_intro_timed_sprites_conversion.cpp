#include "shadowrun_ghidra/title_intro/title_intro_timed_sprites_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void reset_title_intro_timed_sprites(TitleIntroTimedSpriteState& state) {
    state.next_event_frame = 0;
    state.descriptor_index = 6U;
}

void begin_title_intro_timed_sprites(TitleIntroTimedSpriteState& state,
                                      const std::uint16_t x,
                                      const std::uint16_t y) {
    state.next_event_frame = 0;
    state.descriptor_index = 0;
    state.x = x;
    state.y = y;
}

TitleIntroTimedSpriteAdvanceResult advance_title_intro_timed_sprites(
    TitleIntroTimedSpriteState& state,
    const std::array<std::uint32_t, kTitleIntroTimedSpriteCount>& descriptors,
    const TitleIntroTimedSpriteCallbacks& callbacks) {
    if (state.next_event_frame <= state.current_frame) {
        if (state.descriptor_index >= kTitleIntroTimedSpriteCount) {
            return TitleIntroTimedSpriteAdvanceResult::no_op;
        }
        state.current_sprite_descriptor =
            descriptors[state.descriptor_index];
        state.next_event_frame = static_cast<std::uint16_t>(
            state.current_frame + 8U);
        ++state.descriptor_index;
    }

    if (callbacks.draw_sprite) {
        callbacks.draw_sprite(
            {state.current_sprite_descriptor, state.x, state.y, 0x6000U});
    }
    if (state.descriptor_index >= kTitleIntroTimedSpriteCount) {
        return TitleIntroTimedSpriteAdvanceResult::sequence_finished;
    }
    return TitleIntroTimedSpriteAdvanceResult::sprite_rendered;
}

} // namespace shadowrun::ghidra_conversion
