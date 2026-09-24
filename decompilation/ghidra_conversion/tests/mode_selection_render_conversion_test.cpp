#include "shadowrun_ghidra/mode_selection_render_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    ModeSelectionRenderState state{};
    std::vector<std::uint8_t> prepared;
    std::vector<std::uint8_t> uploaded;
    std::vector<int> order;
    std::uint32_t vblanks = 0;
    std::uint32_t latches = 0;
    std::uint32_t world_updates = 0;
    ModeSelectionRenderCallbacks callbacks{};
    callbacks.prepare_panel = [&](std::uint8_t panel) {
        prepared.push_back(panel);
        order.push_back(1);
    };
    callbacks.upload_panel = [&](std::uint8_t panel) {
        uploaded.push_back(panel);
        order.push_back(2);
    };
    callbacks.wait_for_vblank = [&] { ++vblanks; };
    callbacks.latch_controller_events = [&] { ++latches; };
    callbacks.service_world_state = [&] { ++world_updates; };
    callbacks.render_title_text = [&] { order.push_back(3); };
    callbacks.read_final_text_marker = [] { return 0x13U; };

    render_mode_selection_panels(state, callbacks);
    if (state.rendered_panel_count != 4U || state.final_text_marker != 0x13U ||
        prepared != std::vector<std::uint8_t>{0U, 1U, 2U, 3U} ||
        uploaded != prepared || vblanks != 8U || latches != 8U ||
        world_updates != 8U || order.front() != 1 || order.back() != 3) {
        return 1;
    }
    return 0;
}
