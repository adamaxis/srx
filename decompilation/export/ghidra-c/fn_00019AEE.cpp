// Ghidra headless export
// Entry: 0x00019AEE
// Name: refresh_mode_glyph
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid


void refresh_mode_glyph(void)

{
  (&DAT_00ff1113)[(short)(DAT_00fff118 * 0x14)] =
       (&DAT_000e31dd)[(short)((byte)(&DAT_00ff1113)[(short)(DAT_00fff118 * 0x14)] - 0x11)];
  write_repeated_vdp_grid();
  return;
}


