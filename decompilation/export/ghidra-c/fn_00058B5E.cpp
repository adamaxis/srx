// Ghidra headless export
// Entry: 0x00058B5E
// Name: refresh_field_mode_panel_grid
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::refresh_field_mode_panel_grid.
// The source prepares the current selection data, then writes one grid unless
// FFF0B7 is the special result-one mode.
//
// Calls recovered from direct call instructions:
//   0x00057C46 prepare_field_mode_selection_data
//   0x00001232 write_vdp_grid
