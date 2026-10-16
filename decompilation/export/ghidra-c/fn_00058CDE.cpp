// Ghidra headless export
// Entry: 0x00058CDE
// Name: advance_field_mode_panel_cursor
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::advance_field_mode_panel_cursor.
// The source clamps the panel selector to 0..1, resets the primary selector,
// and uses the alternate sprite path or the normal reset-and-redraw path.
//
// Calls recovered from direct call instructions:
//   0x00000E34 reset_vdp_upload_state
//   0x00058B5E refresh_field_mode_panel_grid
//   0x00058BB6 render_field_mode_selection
//   0x00058D32 refresh_field_mode_selection_sprites
