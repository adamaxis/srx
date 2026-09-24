// Ghidra headless export
// Entry: 0x000266E4
// Name: FUN_000266e4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00024E20 FUN_00024e20
//   0x00026696 FUN_00026696


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000266e4(void)

{
  undefined2 *unaff_A5;
  
  FUN_00024e20();
  DAT_00ffe192 = *unaff_A5;
  DAT_00ffe19a = *unaff_A5;
  DAT_00ffe194 = unaff_A5[1];
  DAT_00ffe19c = unaff_A5[1];
  if (*(char *)((int)&DAT_00ff1100 + (int)(short)(DAT_00fff118 * 0x14)) == '\0') {
    DAT_00ffe19e = 0xe;
    DAT_00ffe1a0 = 8;
  }
  else {
    DAT_00ffe19e = 0xc;
    DAT_00ffe1a0 = 10;
  }
  DAT_00ffe196 = 0;
  DAT_00ffe198 = 0;
  DAT_00ffe1a2 = 0;
  DAT_00ffe1a4 = 0;
  DAT_00ffe1a8 = 1;
  DAT_00ffe1a6 = 1;
  FUN_00026696();
  DAT_00ffe1aa = &DAT_00ff8700;
  DAT_00ffe1ae = &DAT_00ff9700;
  FUN_00024e20();
  DAT_00ffe1ae = &DAT_00ffb740;
  FUN_00024e20();
  _DAT_00ffb740 = _DAT_00ffb740 + -1;
  DAT_00ffdd28 = 0x11e;
  DAT_00ffdd2a = 0xe4;
  return;
}


