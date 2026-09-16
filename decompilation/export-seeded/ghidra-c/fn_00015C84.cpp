// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00015C84


int FUN_00015c84(void)

{
  int iVar1;
  undefined2 unaff_D2w;
  int in_A0;
  int extraout_A0;
  bool bVar2;
  
  iVar1 = *(int *)(in_A0 + 0xa2);
  bVar2 = false;
  if (iVar1 != 0) {
    iVar1 = FUN_00015c58();
    if (bVar2) {
      *(undefined2 *)(extraout_A0 + 0x34) = unaff_D2w;
      return iVar1;
    }
  }
  return iVar1;
}


