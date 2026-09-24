// Ghidra headless export
// Entry: 0x0001C866
// Name: FUN_0001c866
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378
//   0x00019E32 FUN_00019e32


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001c866(void)

{
  DAT_00ffe066 = DAT_00ffe066 + -1;
  if (DAT_00ffe066 < 0) {
    DAT_00ffe066 = 3;
    DAT_00ffe064 = DAT_00ffe064 + 1;
    if (0xb < DAT_00ffe064) {
      if (DAT_00ffe060 < 0x120) {
        DAT_00ff1588 = &DAT_0001c288;
        DAT_00ff158c = 0;
      }
      else {
        DAT_00ff1584 = &DAT_0001c28e;
        DAT_00ff158d = 0;
      }
      _DAT_00ffe0d6 = 0;
      FUN_00019e32();
      _DAT_00ffe05e = 0;
      return;
    }
  }
  FUN_00001378();
  return;
}


