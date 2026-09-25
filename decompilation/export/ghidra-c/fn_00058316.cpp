// Ghidra headless export
// Entry: 0x00058316
// Name: FUN_00058316
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text


void FUN_00058316(void)

{
  undefined4 extraout_A1;
  
  if ((short)(DAT_00fff0d0 - 4) < 0) {
    render_encoded_title_text();
    render_encoded_title_text();
    write_vdp_grid(extraout_A1);
  }
  render_encoded_title_text();
  return;
}


