#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

constexpr std::uint8_t kModeSelectionPanelCount = 4U;
constexpr std::uint8_t kModeSelectionVblankPasses = 2U;

struct ModeSelectionRenderState {
    std::uint8_t rendered_panel_count = 0;
    std::uint8_t final_text_marker = 0;
};

struct ModeSelectionRenderCallbacks {
    std::function<void(std::uint8_t)> prepare_panel;
    std::function<void(std::uint8_t)> upload_panel;
    std::function<void()> wait_for_vblank;
    std::function<void()> latch_controller_events;
    std::function<void()> service_world_state;
    std::function<void()> render_title_text;
    std::function<std::uint8_t()> read_final_text_marker;
};

// ROM 0x13726: render the four source mode-selection panels, preserving the
// two-vblank service window after each panel and the trailing title text pass.
// Transfer and text leaves remain host callbacks rather than VDP operations.
void render_mode_selection_panels(
    ModeSelectionRenderState& state,
    const ModeSelectionRenderCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
