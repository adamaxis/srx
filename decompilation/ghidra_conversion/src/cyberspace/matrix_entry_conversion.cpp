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

void prepare_matrix_session_entry(MatrixEntryState& state,
                                  const MatrixEntryCallbacks& callbacks) {
    // 0x181F8..0x1821C: display preparation before the resource copy.
    call_if_present(callbacks.set_display_command,
                    MatrixDisplayCommand::prepare_matrix);
    call_if_present(callbacks.set_display_command,
                    MatrixDisplayCommand::prepare_resource_upload);

    // 0x18208..0x18226: copy E2E3A's source
    // block through the existing host extraction boundary.
    call_if_present(callbacks.copy_resource_block, 0xE2E3AU);
    state.display_dirty_flags = static_cast<std::uint8_t>(
        state.display_dirty_flags | 0x01U);
    call_if_present(callbacks.upload_menu_resource_pair);

    // 0x18220..0x1822A: establish the ten-slot Matrix menu before the
    // fall-through into 0x1822C.
    state.menu_slot_count = 10U;
}

void enter_matrix_screen(MatrixEntryState& state,
                         const MatrixEntryCallbacks& callbacks) {
    prepare_matrix_session_entry(state, callbacks);

    // The caller-specific inline shell clears the overlay marker before the
    // shared Matrix loop and then owns the return-to-field presentation.
    state.overlay_marker = 0;
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
