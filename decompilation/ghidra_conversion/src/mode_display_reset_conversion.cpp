#include "shadowrun_ghidra/mode_display_reset_conversion.hpp"

namespace shadowrun::ghidra_conversion {

void reset_mode_display_buffers(
    ModeDisplayResetState& state,
    const ModeDisplayResetCallbacks& callbacks) {
    if (callbacks.clear_plane_a) {
        callbacks.clear_plane_a();
    }
    if (callbacks.clear_plane_b) {
        callbacks.clear_plane_b();
    }
    if (callbacks.wait_for_vblank) {
        callbacks.wait_for_vblank();
    }
    if (callbacks.reset_upload_state) {
        callbacks.reset_upload_state();
    }
    state.primary_transfer_marker = 0U;
    state.secondary_transfer_marker = 0U;
    if (callbacks.write_transfer_header) {
        callbacks.write_transfer_header();
    }
    if (callbacks.write_fixed_transfer_pair) {
        callbacks.write_fixed_transfer_pair();
    }
}

} // namespace shadowrun::ghidra_conversion
