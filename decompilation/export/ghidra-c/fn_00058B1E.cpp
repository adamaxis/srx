// Ghidra headless export
// Entry: 0x00058B1E
// Name: render_field_mode_grid_pair_batches
// Decompile status: source-shaped annotation
//
// The native conversion is
// ghidra_conversion::render_field_mode_grid_pair_batches(). It preserves the
// six-grid then two-grid source batch boundary.
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid


void render_field_mode_grid_pair_batches(void)

{
  short sVar1;
  
  sVar1 = 5;
  do {
    write_vdp_grid();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  sVar1 = 1;
  do {
    write_vdp_grid();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}

