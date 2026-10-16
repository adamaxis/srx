// Ghidra headless export
// Entry: 0x00001BAC
// Name: finalize_title_input_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void finalize_title_input_sequence(void)

{
  if (((DAT_00fff101 == '\a') && (_DAT_00ffcb40 == 0x40101040)) && (DAT_00ffcb44 == 0x20401000)) {
    DAT_00fff101 = 0xff;
    return;
  }
  DAT_00fff101 = 0;
  return;
}


