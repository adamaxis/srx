// Ghidra headless export
// Entry: 0x00019736
// Name: FUN_00019736
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00019736(void)

{
  if ((-1 < DAT_00fff10c) && ((&DAT_00fff0ed)[(short)(ushort)DAT_00fff0d4] != '\x06')) {
    return;
  }
  (&DAT_00fff0ed)[(short)(ushort)DAT_00fff0d4] = 0;
  write_vdp_grid();
  return;
}


