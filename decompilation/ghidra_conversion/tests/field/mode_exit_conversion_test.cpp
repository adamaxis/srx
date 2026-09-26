#include "shadowrun_ghidra/field/mode_exit_conversion.hpp"

#include <cstdint>
#include <vector>

int main() {
    using namespace shadowrun::ghidra_conversion;

    ModeExitServiceState state{};
    state.pending_dispatch = true;
    std::vector<std::uint16_t> commands;
    std::vector<int> order;
    ModeExitServiceCallbacks callbacks{};
    callbacks.set_display_command = [&](std::uint16_t command) {
        commands.push_back(command);
    };
    callbacks.load_title_font = [&] { order.push_back(1); };
    callbacks.transfer_optional_menu_assets = [&] { order.push_back(2); };
    callbacks.prepare_mode_exit_resources = [&] { order.push_back(3); };
    callbacks.dispatch_pending_mode_service = [&] { order.push_back(4); };
    callbacks.run_mode_text_prompt = [&] { order.push_back(5); };
    callbacks.reset_mode_display_buffers = [&] { order.push_back(6); };
    callbacks.refresh_mode_palette_data = [&] { order.push_back(7); };

    run_mode_exit_services(state, callbacks);
    if (commands != std::vector<std::uint16_t>{0x9100U, 0x9191U} ||
        !state.dispatch_cleared || state.pending_dispatch ||
        state.display_command != 0x9191U || order !=
            std::vector<int>{1, 2, 3, 4, 5, 6, 7}) {
        return 1;
    }
    return 0;
}
