// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005AD1A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005ad1a(void)

{
  short sVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  FUN_0005b29a();
  FUN_00001406();
  _DAT_00fff11e = _DAT_00ffdf88;
  FUN_0005b002();
  FUN_0005b198();
  FUN_0005b0bc();
  FUN_0005a9e2();
  sVar1 = 0x1f;
  puVar2 = (undefined4 *)&DAT_00ffd8fe;
  puVar3 = (undefined4 *)&DAT_00ffd87e;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  FUN_0000195c();
  sVar1 = 0x528;
  do {
    FUN_00000bd2();
    FUN_0000195c();
    FUN_00000bf2();
    if (sVar1 == 0x1e0) {
      func_0x0005a96a();
    }
    FUN_0005b08e();
    FUN_0005b00a();
    FUN_0005b056();
    FUN_0005b0bc();
  } while ((_DAT_00ffddc2 == 0) && (sVar1 = sVar1 + -1, sVar1 != -1));
  return;
}


