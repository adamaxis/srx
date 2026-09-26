#include "shadowrun_ghidra/cyberspace/matrix_entry_conversion.hpp"

namespace shadowrun::ghidra_conversion {
namespace {

template <typename Callback, typename... Args>
void call_if_present(const Callback& callback, Args&&... args) {
    if (callback) {
        callback(static_cast<Args&&>(args)...);
    }
}

} // namespace

void enter_matrix_screen(MatrixEntryState& state,
                         const MatrixEntryCallbacks& callbacks) {
    // 0x1817E..0x1818C: display preparation before the resource copy.
    call_if_present(callbacks.set_display_command,
                    MatrixDisplayCommand::prepare_matrix);
    call_if_present(callbacks.set_display_command,
                    MatrixDisplayCommand::prepare_resource_upload);

    // 0x1818E..0x1819E: clear the overlay marker and copy E2E3A's source
    // block through the existing host extraction boundary.
    state.overlay_marker = 0;
    call_if_present(callbacks.copy_resource_block, 0xE2E3AU);
    state.display_dirty_flags = static_cast<std::uint8_t>(
        state.display_dirty_flags | 0x01U);
    call_if_present(callbacks.upload_menu_resource_pair);

    // 0x181B2..0x181C2: establish the ten-slot Matrix menu and enter 1822C.
    state.menu_slot_count = 10U;
    call_if_present(callbacks.set_display_command,
                    MatrixDisplayCommand::matrix_loop);
    call_if_present(callbacks.run_matrix_loop);

    // 0x181C4..0x181F4: refresh the mode data, restore field presentation,
    // and hand control back through the source tail.
    call_if_present(callbacks.set_display_command,
                    MatrixDisplayCommand::prepare_field_return);
    call_if_present(callbacks.refresh_mode_resource_metadata, 0xD29D6U);
    call_if_present(callbacks.refresh_mode_palette_data);
    call_if_present(callbacks.set_display_command,
                    MatrixDisplayCommand::field_return);
    call_if_present(callbacks.upload_field_asset_sequence);
    call_if_present(callbacks.refresh_field_animation_assets);
    call_if_present(callbacks.return_to_field_mode);
    state.returned_to_field = true;
}

} // namespace shadowrun::ghidra_conversion
