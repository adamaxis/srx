// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005D260


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005d260(void)

{
  short sVar1;
  undefined4 *puVar2;
  
  sVar1 = 0x10;
  puVar2 = (undefined4 *)&DAT_00fff11e;
  do {
    *puVar2 = 0;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  _DAT_00fff11e = 0x11c0;
  _DAT_00fff122 = 0xfe80;
  _DAT_00fff140 = 0xffff;
  _DAT_00fff142 = 0xb4;
  return;
}


