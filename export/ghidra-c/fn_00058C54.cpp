// Ghidra headless export
// Entry: 0x00058C54
// Name: advance_field_mode_primary_cursor
// Decompile status: source-shaped annotation
//
// Native conversion: ghidra_conversion::advance_field_mode_primary_cursor.
// A signed delta is clamped to 0..3 for result-zero normal mode, 0..5 for
// other normal results, or 0..7 for the alternate panel. The source then
// refreshes the corresponding normal or sprite-backed panel presentation.
//
// Calls recovered from direct call instructions:
//   0x00058B5E refresh_field_mode_panel_grid
//   0x00058BB6 render_field_mode_selection
//   0x00058D32 refresh_field_mode_selection_sprites
