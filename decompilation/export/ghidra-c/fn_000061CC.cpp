// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000061CC


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000061cc(void)

{
  undefined4 *puVar1;
  short in_D0w;
  int unaff_A2;
  
  puVar1 = _DAT_00ffdf32;
  *_DAT_00ffdf32 = *(undefined4 *)(unaff_A2 + 2);
  *(undefined2 *)(puVar1 + 1) = *(undefined2 *)(unaff_A2 + 6);
  *(short *)((int)puVar1 + 6) = in_D0w << 5;
  _DAT_00ffdf32 = (undefined4 *)CONCAT31(_DAT_00ffdf32,DAT_00ffdf35 + '\b');
  _DAT_00ffdf30 = _DAT_00ffdf30 + 1;
  return;
}


