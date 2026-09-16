// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00015930


void FUN_00015930(void)

{
  undefined2 uVar1;
  short sVar2;
  undefined2 extraout_D1w;
  short extraout_D1w_00;
  int extraout_A1;
  undefined1 in_CF;
  
  uVar1 = thunk_FUN_0001a2cc();
  if (!(bool)in_CF) {
    *(undefined1 *)(extraout_A1 + 0xb2) = 2;
    *(undefined2 *)(extraout_A1 + 0x32) = uVar1;
    *(undefined2 *)(extraout_A1 + 0x2a) = extraout_D1w;
    *(undefined1 *)(extraout_A1 + 2) = 0;
    *(undefined1 *)(extraout_A1 + 3) = 0;
    sVar2 = 2;
    if (sRam0000494a != 0) {
      FUN_000530de();
      sVar2 = extraout_D1w_00;
    }
    (**(code **)((int)&PTR_LAB_00004964 + (int)(short)(sVar2 << 2)))();
  }
  return;
}


