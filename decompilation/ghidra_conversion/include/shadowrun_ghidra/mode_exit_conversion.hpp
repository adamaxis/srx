#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

struct ModeExitServiceState {
    bool pending_dispatch = false;
    bool dispatch_cleared = false;
    std::uint16_t display_command = 0;
};

struct ModeExitServiceCallbacks {
    std::function<void(std::uint16_t)> set_display_command;
    std::function<void()> load_title_font;
    std::function<void()> transfer_optional_menu_assets;
    std::function<void()> prepare_mode_exit_resources;
    std::function<void()> dispatch_pending_mode_service;
    std::function<void()> run_mode_text_prompt;
    std::function<void()> reset_mode_display_buffers;
    std::function<void()> refresh_mode_palette_data;
};

// ROM 0x1F246: coordinate the mode-exit resource setup and restore the final
// display command. The prompt/resource implementations remain callbacks.
void run_mode_exit_services(ModeExitServiceState& state,
                            const ModeExitServiceCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
