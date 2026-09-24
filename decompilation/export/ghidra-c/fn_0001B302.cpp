// Ghidra headless export
// Entry: 0x0001B302
// Name: FUN_0001b302
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D44 advance_source_prng
//   0x0001B5A6 FUN_0001b5a6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001b302(void)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  short sVar4;
  short *extraout_A0;
  
  DAT_00ffe086 = DAT_00ffe086 + 1;
  if (DAT_00ffe086 < 0x3c) {
    sVar4 = 0x57;
    do {
      uVar1 = advance_source_prng();
      uVar2 = uVar1 & 0x1f;
      if ((uVar1 & 0x80) != 0) {
        uVar2 = -uVar2;
      }
      uVar3 = uVar1 >> 8 & 0x1f;
      if ((uVar1 >> 8 & 0x80) != 0) {
        uVar3 = -uVar3;
      }
      *extraout_A0 = uVar2 + *extraout_A0;
      extraout_A0[1] = uVar2 + extraout_A0[1];
      extraout_A0[2] = uVar3 + extraout_A0[2];
      extraout_A0[3] = uVar3 + extraout_A0[3];
      sVar4 = sVar4 + -1;
    } while (sVar4 != -1);
    FUN_00000e18();
    DAT_00ffef14 = DAT_00ffef14 | 0xc;
    return;
  }
  _DAT_00ffe084 = 0;
  FUN_0001b5a6();
  return;
}


