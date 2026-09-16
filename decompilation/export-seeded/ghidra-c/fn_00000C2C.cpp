// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00000C2C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00000c2c(void)

{
  int in_D1;
  short sVar1;
  int *piVar2;
  undefined1 *puVar3;
  
  if (_DAT_00fff9e8 == 0x12345678) {
    sVar1 = -0xe19;
    puVar3 = &DAT_00ff0000;
    do {
      *puVar3 = 0;
      sVar1 = sVar1 + -1;
      puVar3 = puVar3 + 1;
    } while (sVar1 != -1);
  }
  else {
    sVar1 = 0x3fdf;
    piVar2 = (int *)&DAT_00ff0000;
    do {
      in_D1 = *piVar2 + in_D1;
      *piVar2 = 0;
      sVar1 = sVar1 + -1;
      piVar2 = piVar2 + 1;
    } while (sVar1 != -1);
    _DAT_00fff9f8 = (undefined2)in_D1;
  }
  FUN_00000ec0();
  FUN_00000dfc();
  _DAT_00ffdd02 = 1;
  FUN_00000e34();
  FUN_00000cec();
  FUN_00000e9c();
  sVar1 = 7;
  do {
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  _DAT_00c00004 = 0x40000010;
  _DAT_00c00000 = 0;
  FUN_000531aa();
  FUN_000531aa();
  FUN_000531aa();
  FUN_000531aa();
  FUN_000531aa();
  return;
}


