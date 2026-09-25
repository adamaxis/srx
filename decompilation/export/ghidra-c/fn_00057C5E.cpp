// Ghidra headless export
// Entry: 0x00057C5E
// Name: refresh_field_action_detail_panel
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid
//
// The caller supplies the source panel parameters in D4/D1. At this entry the
// detail panel is the five-row block at tile origin 0xC484.

void refresh_field_action_detail_panel(void)
{
  write_repeated_vdp_grid(
      0xC484, // tile origin
      5,      // repeated rows
      0);     // source panel bank
}
