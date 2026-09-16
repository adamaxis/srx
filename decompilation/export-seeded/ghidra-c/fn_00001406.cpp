// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00001406


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00001406(void)

{
  uint uVar1;
  int unaff_A6;
  uint *puVar2;
  uint *puVar3;
  
  FUN_0000131c();
  FUN_00000cec();
  FUN_00000ffe();
  FUN_0000102a();
  FUN_00000e34();
  FUN_00001368();
  _DAT_00ffdd08 = *(undefined2 *)(unaff_A6 + 8);
  _DAT_00ffdd0c = *(undefined2 *)(unaff_A6 + 10);
  _DAT_00ffdd0a = *(undefined2 *)(unaff_A6 + 0xc);
  puVar2 = (uint *)(unaff_A6 + 0x10);
  _DAT_00ffdd0e = *(undefined2 *)(unaff_A6 + 0xe);
  while( true ) {
    puVar3 = puVar2 + 1;
    if (*puVar2 == 0) break;
    if ((*puVar2 & 1) == 0) {
      puVar2 = puVar2 + 3;
      FUN_00000f5c();
    }
    else {
      puVar2 = puVar2 + 3;
      FUN_00001530();
    }
  }
  while (uVar1 = *puVar3, uVar1 != 0) {
    puVar3 = (uint *)((int)puVar3 + 0xe);
    if ((uVar1 & 1) == 0) {
      FUN_00001232();
    }
    else {
      FUN_00001728();
    }
  }
  DAT_00ffef14 = DAT_00ffef14 | 0xd;
  FUN_00000bd2();
  return;
}


