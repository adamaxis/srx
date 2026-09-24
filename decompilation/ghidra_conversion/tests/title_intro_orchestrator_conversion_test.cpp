#include "shadowrun_ghidra/title_intro_orchestrator_conversion.hpp"

#include <array>
#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    TitleIntroOrchestratorAssets assets{};
    assets.text_assets = {{1U, 2U, 3U}};
    assets.first_graphics_asset = 10U;
    assets.second_graphics_asset = 11U;
    assets.palette_asset = 12U;
    assets.hud_sprite_asset = 13U;
    const std::array<TitleIntroStageTimedPhase,
                     kTitleIntroStageTimedPhaseCount>
        phases{{{20U, 1U, 2U, 3U}, {21U, 4U, 5U, 3U},
                {22U, 6U, 7U, 3U}}};

    TitleIntroOrchestratorState state{};
    std::vector<int> order;
    std::vector<TitleIntroSourceStreamRequest> streams;
    std::vector<TitleIntroFillRowRequest> fill_rows;
    std::vector<std::uint16_t> service_steps;
    std::uint32_t text_count = 0;
    std::uint32_t hud_count = 0;
    std::uint32_t fades = 0;
    TitleIntroOrchestratorCallbacks callbacks{};
    callbacks.reset_upload_state = [&] { order.push_back(1); };
    callbacks.clear_display_buffer = [&](const auto&) { order.push_back(2); };
    callbacks.clear_plane_a = [&] { order.push_back(3); };
    callbacks.clear_plane_b = [&] { order.push_back(4); };
    callbacks.decode_and_upload = [&](const auto& request) {
        streams.push_back(request);
        order.push_back(5);
    };
    callbacks.load_title_font = [&] { order.push_back(6); };
    callbacks.render_text = [&](const auto&) { ++text_count; };
    callbacks.wait_for_vblank = [&] {
        ++state.current_frame;
        order.push_back(7);
    };
    callbacks.copy_target_palette = [&](std::uint32_t asset) {
        if (asset != 12U) {
            order.push_back(99);
        }
    };
    callbacks.write_fill_row = [&](const auto& row) { fill_rows.push_back(row); };
    callbacks.render_hud_sprites = [&](const auto& request) {
        if (request.asset == 13U && request.sprite_word_count == 0x28U &&
            request.row_count == 0x0EU) {
            ++hud_count;
        }
    };
    callbacks.advance_palette_fade = [&] { ++fades; };
    callbacks.sample_controller = [&] { order.push_back(8); };
    callbacks.latch_controller_events = [&] { order.push_back(9); };
    callbacks.service_step = [&](std::uint16_t step) {
        service_steps.push_back(step);
    };
    callbacks.draw_sprite = [](const auto&) {};

    if (run_title_intro_orchestrator(state, assets, phases, 30U, 31U,
                                     callbacks) !=
            TitleIntroOrchestratorStopReason::source_complete ||
        streams.size() != 2U || streams[0].decoded_bytes != 0x3020U ||
        streams[1].decoded_bytes != 0xC000U || text_count != 3U ||
        fill_rows.size() != 13U || hud_count != 1U || fades != 1U ||
        service_steps.size() != 52U || state.loop_frame != 0x115U ||
        state.frame_state.scene_mode != 1U ||
        state.frame_state.dirty_flags != 0x02U || order.size() < 9U ||
        order[0] != 1 || order[1] != 2 || order[2] != 3 || order[3] != 4 ||
        order[4] != 5 || order[5] != 5 || order[6] != 6) {
        return 1;
    }
    return 0;
}
