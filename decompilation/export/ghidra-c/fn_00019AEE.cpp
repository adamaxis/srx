// Ghidra headless export
// Entry: 0x00019AEE
// Name: FUN_00019aee
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 write_repeated_vdp_grid


void FUN_00019aee(void)

{
  (&DAT_00ff1113)[(short)(DAT_00fff118 * 0x14)] =
       (&DAT_000e31dd)[(short)((byte)(&DAT_00ff1113)[(short)(DAT_00fff118 * 0x14)] - 0x11)];
  write_repeated_vdp_grid();
  return;
}


