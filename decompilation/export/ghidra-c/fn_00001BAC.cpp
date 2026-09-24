// Ghidra headless export
// Entry: 0x00001BAC
// Name: FUN_00001bac
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00001bac(void)

{
  if (((DAT_00fff101 == '\a') && (_DAT_00ffcb40 == 0x40101040)) && (DAT_00ffcb44 == 0x20401000)) {
    DAT_00fff101 = 0xff;
    return;
  }
  DAT_00fff101 = 0;
  return;
}


