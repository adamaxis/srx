// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00001E4E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_00001e4e(void)

{
  ushort uVar1;
  short sVar2;
  undefined2 *puVar3;
  undefined2 *puVar4;
  
  uVar1 = _DAT_00fff1e6 & 1;
  if (uVar1 == 0) {
    uVar1 = FUN_000530de();
  }
  if (_DAT_00fff1e6 < 0x26) {
    uVar1 = FUN_0005c8c6();
    _DAT_00fff1e6 = _DAT_00fff1e6 + 1;
    _DAT_00fff1e2 = _DAT_00fff1e2 + 2;
  }
  if (_DAT_00fff1e6 == 0x25) {
    _DAT_00ffdd5a = 1;
    sVar2 = 0xf;
    puVar3 = &DAT_00002070;
    puVar4 = (undefined2 *)&DAT_00ffd93e;
    do {
      *puVar4 = *puVar3;
      sVar2 = sVar2 + -1;
      puVar3 = puVar3 + 1;
      puVar4 = puVar4 + 1;
    } while (sVar2 != -1);
  }
  return uVar1;
}


