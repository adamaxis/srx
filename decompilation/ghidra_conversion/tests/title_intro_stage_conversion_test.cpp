#include "shadowrun_ghidra/title_intro_stage_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    const std::array<std::uint32_t, kTitleIntroStageTextCount> text_assets{{
        0xA000U, 0xA001U, 0xA002U}};
    std::vector<TitleIntroStageTextRequest> text_requests;
    render_title_intro_stage_text_batch(
        text_assets,
        [&text_requests](const TitleIntroStageTextRequest& request) {
            text_requests.push_back(request);
        });
    if (text_requests.size() != 3U ||
        text_requests[0].destination_word != 0x2186U ||
        text_requests[2].destination_word != 0x2386U ||
        text_requests[1].palette_flags != 0x8000U) {
        return 1;
    }

    TitleIntroStageProjectionState projection{0x1200U, 0x3400U};
    reset_title_intro_stage_projection(projection);
    std::vector<TitleIntroStageSpriteRequest> projection_requests;
    const auto capture_projection =
        [&projection_requests](const TitleIntroStageSpriteRequest& request) {
            projection_requests.push_back(request);
        };
    draw_title_intro_stage_base_sprite(0xB000U, capture_projection);
    advance_title_intro_stage_projection(projection, 0xB001U,
                                          capture_projection);
    if (projection_requests.size() != 2U ||
        projection_requests[0].x != 0x00C2U ||
        projection_requests[0].palette_flags != 0xC000U ||
        projection_requests[1].x != 0x0060U || projection_requests[1].y !=
            0x0123U || projection.x_accumulator != 0x0040U ||
        projection.y_accumulator != 0x0018U) {
        return 2;
    }

    TitleIntroStageTimedState timed{0U, 0U, 0U, 0U, 0U, 0U, 0U};
    reset_title_intro_stage_timed_sprites(timed);
    const std::array<TitleIntroStageTimedPhase,
                     kTitleIntroStageTimedPhaseCount>
        phases{{{0xC000U, 0x0100U, 0x0120U, 6U},
                {0xC001U, 0x0101U, 0x0121U, 6U},
                {0xC002U, 0x0102U, 0x0122U, 6U}}};
    std::vector<TitleIntroStageSpriteRequest> timed_requests;
    const auto capture_timed =
        [&timed_requests](const TitleIntroStageSpriteRequest& request) {
            timed_requests.push_back(request);
        };
    timed.current_frame = 0x003BU;
    advance_title_intro_stage_timed_sprites(timed, phases, capture_timed);
    if (!timed_requests.empty()) {
        return 3;
    }
    timed.current_frame = 0x003CU;
    advance_title_intro_stage_timed_sprites(timed, phases, capture_timed);
    if (timed.current_sprite_descriptor != 0xC000U || timed.phase != 1U ||
        timed_requests.front().palette_flags != 0xE000U) {
        return 4;
    }

    std::vector<std::uint16_t> service_order;
    if (!advance_title_intro_stage_service_steps(
            0x00B4U, 0U,
            [&service_order](const std::uint16_t index) {
                service_order.push_back(index);
            }) ||
        service_order != std::vector<std::uint16_t>{0U, 1U, 2U, 3U} ||
        advance_title_intro_stage_service_steps(0x00B5U, 0U, {})) {
        return 5;
    }
    return 0;
}
