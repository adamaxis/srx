#include "shadowrun_ghidra/title_intro/title_intro_transfer_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroDisplayClearRequest clear_request{};
    clear_title_intro_display_buffer(
        [&clear_request](const auto& request) { clear_request = request; });
    if (clear_request.transfer_command != 0x3000U ||
        clear_request.packed_fill_word != 0x07FF07FFU ||
        clear_request.long_write_count != 0x0400U) {
        return 1;
    }

    std::vector<TitleIntroFillRowRequest> rows;
    fill_title_intro_word_block(
        {0x0786U, 20U, 13U, 0x0080U, 0x07FFU},
        [&rows](const auto& request) { rows.push_back(request); });
    if (rows.size() != 13U || rows.front().destination_offset != 0x0786U ||
        rows.back().destination_offset != 0x0D86U ||
        rows.front().word_count != 20U || rows.back().row_index != 12U ||
        rows.front().fill_word != 0x07FFU) {
        return 2;
    }

    WorldFrameServiceState frame{};
    frame.words[3] = 0x1111U;
    reset_world_frame_service_state(frame);
    if (frame.words[0] != 0x11C0U || frame.words[1] != 0U ||
        frame.words[2] != 0xFE80U || frame.words[3] != 0U ||
        frame.words[0x11] != 0xFFFFU || frame.words[0x12] != 0x00B4U) {
        return 3;
    }

    frame.words[3] = 1U;
    frame.words[5] = 10U;
    frame.words[6] = 20U;
    frame.words[0x0B] = 8U;
    frame.words[0x0C] = 0U;
    auto sprite = advance_world_scene_effect_timer_a(frame);
    if (frame.words[3] != 0U || frame.words[0x0B] != 8U || sprite.x != 12 ||
        sprite.y != 61 || sprite.flags != 0x4000U ||
        sprite.source != 0x0001FEA52U) {
        return 4;
    }
    sprite = advance_world_scene_effect_timer_a(frame);
    if (frame.words[3] != 0x14U || frame.words[0x0B] != 0U ||
        sprite.source != 0x0001FEB38U) {
        return 5;
    }

    frame.words[0x0D] = 0U;
    frame.words[0x0E] = 9U;
    sprite = advance_world_scene_effect_timer_b(frame);
    if (frame.words[0x0D] != 0x0AU || frame.words[0x0E] != 0U ||
        sprite.y != 37 || sprite.source != 0x0001FEADEU) {
        return 6;
    }

    frame.words[0x10] = 0U;
    frame.words[0x0F] = 39U;
    sprite = advance_world_scene_effect_timer_c(frame);
    if (frame.words[0x10] != 4U || frame.words[0x0F] != 0U || sprite.x != 30 ||
        sprite.y != 109 || sprite.source != 0x0001FEC34U) {
        return 7;
    }

    const auto entities = render_title_intro_scene_entities(100, 200, 3U);
    if (entities.size() != 2U || entities[0].y != 106 || entities[0].x != 185 ||
        entities[0].size != 4U || entities[0].sequence != 3U ||
        entities[0].attributes != 0x24F6U || entities[1].y != 101 ||
        entities[1].x != 193 || entities[1].sequence != 4U ||
        entities[1].attributes != 0x24F8U) {
        return 8;
    }
    return 0;
}
