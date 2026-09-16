// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00000D3A


int FUN_00000d3a(void)

{
  uint uVar1;
  ushort extraout_D1w;
  
  uVar1 = FUN_00000d44();
  return (int)(short)((uVar1 & 0xffff) * (uint)extraout_D1w >> 0x10);
}


