// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001E69C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001e69c(void)

{
  short sVar1;
  short sVar2;
  int extraout_A0;
  int extraout_A1;
  int extraout_A1_00;
  undefined6 uVar3;
  
  DAT_00fff0b6 = 0;
  FUN_0001e68c();
  DAT_00fffbec = DAT_00fff0ae;
  uVar3 = FUN_00000d3a();
  sVar1 = (short)CONCAT31((int3)((uint6)uVar3 >> 8),DAT_00fff0ae) * 0x30 +
          (short)((uint6)uVar3 >> 0x20) * 6;
  DAT_00fffbed = *(byte *)(extraout_A1 + sVar1);
  DAT_00fffbf3 = *(undefined1 *)(extraout_A1 + 1 + (int)sVar1);
  sVar2 = FUN_00000d3a();
  _DAT_00fffbf4 = *(short *)(extraout_A1_00 + 2 + (int)sVar1) + sVar2 * 5;
  _DAT_00fff0c0 = *(undefined2 *)(extraout_A1_00 + 2 + (int)sVar1);
  (**(code **)((uint)DAT_00fffbed * 4 + extraout_A0))();
  return;
}


