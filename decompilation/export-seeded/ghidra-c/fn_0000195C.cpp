// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000195C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000195c(void)

{
  ushort uVar1;
  ushort uVar2;
  
  uVar2 = FUN_000019f2();
  uVar1 = _DAT_00ffdd20;
  _DAT_00ffdd20 = uVar2 & 0xff;
  _DAT_00ffdd22 = _DAT_00ffdd20 | ((_DAT_00ffdd20 ^ uVar1) & _DAT_00ffdd20) << 8;
  _DAT_00ffdd04 = *(undefined2 *)(&DAT_000019d2 + (short)((uVar2 & 0xf) << 1));
  return;
}


