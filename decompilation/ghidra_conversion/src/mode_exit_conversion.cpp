#include "shadowrun_ghidra/mode_exit_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void run_mode_exit_services(ModeExitServiceState& state,
                            const ModeExitServiceCallbacks& callbacks) {
    state.display_command = 0x9100U;
    if (callbacks.set_display_command) {
        callbacks.set_display_command(state.display_command);
    }
    if (callbacks.load_title_font) {
        callbacks.load_title_font();
    }
    if (callbacks.transfer_optional_menu_assets) {
        callbacks.transfer_optional_menu_assets();
    }
    if (callbacks.prepare_mode_exit_resources) {
        callbacks.prepare_mode_exit_resources();
    }
    if (state.pending_dispatch && callbacks.dispatch_pending_mode_service) {
        callbacks.dispatch_pending_mode_service();
    }
    state.pending_dispatch = false;
    state.dispatch_cleared = true;
    if (callbacks.run_mode_text_prompt) {
        callbacks.run_mode_text_prompt();
    }
    if (callbacks.reset_mode_display_buffers) {
        callbacks.reset_mode_display_buffers();
    }
    if (callbacks.refresh_mode_palette_data) {
        callbacks.refresh_mode_palette_data();
    }
    state.display_command = 0x9191U;
    if (callbacks.set_display_command) {
        callbacks.set_display_command(state.display_command);
    }
}

} // namespace shadowrun::ghidra_conversion
