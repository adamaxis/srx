#pragma once

#include <cstdint>
#include <functional>

namespace shadowrun::ghidra_conversion {

enum class MatrixDisplayCommand : std::uint16_t {
    prepare_matrix = 0x8134U,
    prepare_resource_upload = 0x9100U,
    prepare_field_return = 0x8134U,
    field_return = 0x9191U,
};

struct MatrixEntryState {
    // The source entry prelude writes ten into FFFBCA before entering the
    // larger Matrix loop at 0x1822C.
    std::uint8_t menu_slot_count = 0;
    // FFEF14 bit 0 is the source display/resource-dirty marker.
    std::uint8_t display_dirty_flags = 0;
    // 0x1817E clears FFF0B7 before the Matrix loop.
    std::uint8_t overlay_marker = 0;
    bool returned_to_field = false;
};

struct MatrixEntryCallbacks {
    std::function<void(MatrixDisplayCommand)> set_display_command;
    std::function<void(std::uint32_t source_address)> copy_resource_block;
    std::function<void()> upload_menu_resource_pair;
    // 0x1822C is the actual Matrix program/deck loop. Keep it as a typed
    // boundary until that larger routine is converted independently.
    std::function<void()> run_matrix_loop;
    std::function<void(std::uint32_t source_address)>
        refresh_mode_resource_metadata;
    std::function<void()> refresh_mode_palette_data;
    std::function<void()> upload_field_asset_sequence;
    std::function<void()> refresh_field_animation_assets;
    // The final 0x20A80 tail returns control to the caller's mode path.
    std::function<void()> return_to_field_mode;
};

// ROM 0x181F8: the confirmed Matrix-session entry prelude. It selects the
// two source display commands, copies the E2E3A resource block, publishes the
// display-dirty bit, uploads the menu resource pair, and establishes the ten
// loaded-program slots. The fall-through into 0x1822C remains a callback
// boundary; this routine does not emulate VDP state.
void prepare_matrix_session_entry(MatrixEntryState& state,
                                  const MatrixEntryCallbacks& callbacks);

// Inline deck-present shell reached from 0x18150 when FF018F bit 0 says a
// cyberdeck is present. This path shares the confirmed 0x181F8 resource
// prelude, then performs its caller-specific return-to-field presentation;
// it intentionally does not emulate VDP state or implement the 0x1822C loop.
void enter_matrix_screen(MatrixEntryState& state,
                         const MatrixEntryCallbacks& callbacks);

} // namespace shadowrun::ghidra_conversion
