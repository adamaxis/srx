// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005AF62


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005af62(void)

{
  short sVar1;
  short unaff_D7w;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined1 uVar4;
  
  _DAT_00ffdd08 = 0;
  _DAT_00ffdd0c = 0;
  _DAT_00ffdd0a = 0;
  _DAT_00ffdd0e = 0;
  DAT_00ffef14 = DAT_00ffef14 | 0xc;
  FUN_0005b29a();
  FUN_00001406();
  FUN_0000c0c2();
  _DAT_00ffdd32 = &DAT_0005add0;
  _DAT_00ffdd36 = DAT_0005add0;
  sVar1 = 0x1f;
  puVar2 = (undefined4 *)&DAT_00ffd8fe;
  puVar3 = (undefined4 *)&DAT_00ffd87e;
  do {
    *puVar3 = *puVar2;
    uVar4 = 0;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  FUN_0000195c();
  while( true ) {
    FUN_00000bd2();
    FUN_0000195c();
    FUN_00000bf2();
    FUN_0005af2a();
    if ((bool)uVar4) {
      return;
    }
    uVar4 = 0;
    if (_DAT_00ffddc2 != 0) break;
    unaff_D7w = unaff_D7w + -1;
    if (unaff_D7w == -1) {
      return;
    }
  }
  return;
}


