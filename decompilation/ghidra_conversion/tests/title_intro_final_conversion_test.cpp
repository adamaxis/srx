#include "shadowrun_ghidra/title_intro_final_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroFinalProjectionState state{0U, 0U, 0U, 0U, 0U, 0U};
    reset_title_intro_final_projection(state);
    if (state.next_event_frame != 0U || state.phase != 0U ||
        state.x != 0x0193U) {
        return 1;
    }
    const std::array<TitleIntroFinalProjectionPhase,
                     kTitleIntroFinalProjectionPhaseCount>
        phases{{{0x8000U, 3U}, {0x8001U, 3U}, {0x8002U, 3U}}};
    std::vector<TitleIntroFinalSpriteRequest> sprites;
    const auto draw_sprite =
        [&sprites](const TitleIntroFinalSpriteRequest& request) {
            sprites.push_back(request);
        };
    state.current_frame = 5U;
    advance_title_intro_final_projection(state, phases, draw_sprite);
    if (!sprites.empty()) {
        return 2;
    }
    state.current_frame = 6U;
    advance_title_intro_final_projection(state, phases, draw_sprite);
    if (state.sprite_descriptor != 0x8000U || state.phase != 1U ||
        state.next_event_frame != 9U || state.x != 0x0197U ||
        sprites.front().y != 0x013CU) {
        return 3;
    }
    state.current_frame = 7U;
    advance_title_intro_final_projection(state, phases, draw_sprite);
    if (sprites.size() != 2U || sprites.back().sprite_descriptor != 0x8000U) {
        return 4;
    }

    const std::array<std::uint32_t, kTitleIntroTextBatchCount> text_assets{{
        0x9000U, 0x9001U, 0x9002U, 0x9003U, 0x9004U, 0x9005U}};
    std::vector<TitleIntroTextRenderRequest> text_requests;
    render_title_intro_text_batch(
        text_assets,
        [&text_requests](const TitleIntroTextRenderRequest& request) {
            text_requests.push_back(request);
        });
    if (text_requests.size() != 6U || text_requests[0].destination_word !=
            0x2906U || text_requests[5].destination_word != 0x2C06U ||
        text_requests[3].text_asset != 0x9003U ||
        text_requests[0].palette_flags != 0xE000U) {
        return 5;
    }
    return 0;
}
