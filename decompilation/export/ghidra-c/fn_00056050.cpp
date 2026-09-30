// Ghidra headless export
// Entry: 0x00056050
// Name: resolve_title_world_action_marker
// Decompile status: source-shaped annotation
// Original Ghidra output did not create a function boundary for this direct
// call target.  The canonical ExportShadowrun.java plugin now creates the
// 0x56050..0x560A9 boundary on regeneration.
//
// Native conversion: ghidra_conversion::resolve_title_world_action_marker.
// The typed implementation preserves the source D6/D4 roles, the
// FFFBBE+0x0A minimum difficulty, the live-record SUB.B, and the raw D4 byte
// that the 0x1A030 caller stores in FFF0AA.
//
// Calls recovered from direct ROM instructions:
//   0x00019BF6 select_hud_mode_auxiliary_value
//   0x00000DBA count_thresholded_random_rolls

void resolve_title_world_action_marker(void)
{
  select_hud_mode_auxiliary_value();
  count_thresholded_random_rolls();
}
