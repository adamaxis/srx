// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000AE6E


void FUN_0000ae6e(void)

{
  undefined2 uVar1;
  uint extraout_D1;
  int in_A0;
  uint extraout_A0;
  bool bVar2;
  
  if (DAT_00ffe186 == '\x03') {
    DAT_00fff0dd = 0;
    return;
  }
  if (*(int *)(in_A0 + 0xa2) != 0) {
    uVar1 = *(undefined2 *)(*(int *)(in_A0 + 0xa2) + 4);
    FUN_00015d3c();
    *(undefined2 *)(extraout_A0 + 0x2e) = uVar1;
    bVar2 = extraout_D1 < extraout_A0;
    if ((extraout_D1 != extraout_A0) && (FUN_00014d8e(), bVar2)) {
      DAT_00fff0dd = 0;
      return;
    }
  }
  FUN_0001033e();
  return;
}


