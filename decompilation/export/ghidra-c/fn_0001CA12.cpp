// Ghidra headless export
// Entry: 0x0001CA12
// Name: FUN_0001ca12
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378
//   0x00019E32 FUN_00019e32


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ca12(void)

{
  DAT_00ffe070 = DAT_00ffe070 + -1;
  if (DAT_00ffe070 < 0) {
    _DAT_00ffe06e = 0;
    _DAT_00ffe0d6 = 0;
    FUN_00019e32();
    return;
  }
  DAT_00ffe072 = DAT_00ffe072 + 1;
  DAT_00ffe06a = DAT_00ffe06c + DAT_00ffe06a;
  DAT_00ffe068 = DAT_00ffe068 + -0x1a;
  FUN_00001378();
  return;
}


