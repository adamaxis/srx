// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005AA96


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005aa96(void)

{
  undefined2 uVar1;
  short sVar2;
  
  FUN_00000e34();
  FUN_00000ffe();
  FUN_0000102a();
  FUN_00001530();
  FUN_00001530();
  FUN_0000c0c2();
  FUN_0005aa5a();
  FUN_00000bd2();
  FUN_00001368();
  FUN_0005b32c();
  _DAT_00fff11e = _DAT_00ffdf88;
  FUN_0005ac28();
  FUN_0005ac70();
  FUN_0005abd2();
  FUN_00000bd2();
  FUN_00001728();
  uVar1 = _DAT_00ffdf88;
  FUN_000012f6();
  _DAT_00ffdf88 = uVar1;
  FUN_0000195c();
  sVar2 = 0x114;
  do {
    FUN_00000bd2();
    FUN_0000195c();
    FUN_00000bf2();
    FUN_0005abd2();
    FUN_0005ab96();
    if (_DAT_00ffddc2 != 0) {
      return;
    }
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


