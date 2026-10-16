#include "shadowrun_ghidra/title_intro/title_intro_final_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void reset_title_intro_final_projection(TitleIntroFinalProjectionState& state) {
    state.next_event_frame = 0;
    state.phase = 0;
    state.x = 0x0193U;
}

void advance_title_intro_final_projection(
    TitleIntroFinalProjectionState& state,
    const std::array<TitleIntroFinalProjectionPhase,
                     kTitleIntroFinalProjectionPhaseCount>& phases,
    const std::function<void(const TitleIntroFinalSpriteRequest&)>& draw_sprite) {
    if (state.current_frame >= state.next_event_frame) {
        const auto elapsed = static_cast<std::uint16_t>(
            state.current_frame - state.animation_start_frame);
        if (elapsed < 6U || state.phase >= kTitleIntroFinalProjectionPhaseCount) {
            return;
        }
        const auto& phase = phases[state.phase];
        state.sprite_descriptor = phase.sprite_descriptor;
        state.next_event_frame = static_cast<std::uint16_t>(
            state.current_frame + phase.delay_frames);
        ++state.phase;
    }

    state.x = static_cast<std::uint16_t>(state.x + 4U);
    if (draw_sprite) {
        draw_sprite({state.sprite_descriptor, state.x, 0x013CU, 0x6000U});
    }
}

void render_title_intro_text_batch(
    const std::array<std::uint32_t, kTitleIntroTextBatchCount>& text_assets,
    const std::function<void(const TitleIntroTextRenderRequest&)>& render_text) {
    if (!render_text) {
        return;
    }
    constexpr std::array<std::uint16_t, kTitleIntroTextBatchCount>
        kDestinationWords{{0x2906U, 0x2986U, 0x2A06U, 0x2A86U, 0x2B86U,
                           0x2C06U}};
    for (std::size_t index = 0; index < text_assets.size(); ++index) {
        render_text({text_assets[index], kDestinationWords[index], 0xE000U});
    }
}

} // namespace shadowrun::ghidra_conversion
