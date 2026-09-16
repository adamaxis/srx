// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00003F9E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00003f9e(void)

{
  int iVar1;
  int extraout_D1;
  short unaff_D3w;
  
  FUN_00003fd8();
  iVar1 = extraout_D1 + *(short *)(&DAT_00ff2700 + unaff_D3w) * 0x20;
  return CONCAT22((short)((uint)iVar1 >> 0x10),
                  ((*(ushort *)(_DAT_00ffddde + (short)iVar1) & 0x7ff) - 0x201) * 0x20);
}


