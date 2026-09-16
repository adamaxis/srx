// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00001910


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00001910(void)

{
  undefined4 in_D0;
  undefined1 *puVar1;
  
  _DAT_00c00004 = 0xc05e0000;
  _DAT_00fff9f0 = _DAT_00fff9f0 + 1;
  if ((_DAT_00fff9f0 & 1) == 0) {
    puVar1 = &DAT_00ffd8dc;
  }
  else {
    puVar1 = &DAT_00ffd8bc;
  }
  _DAT_00c00000 = *(undefined2 *)(puVar1 + 2);
  return in_D0;
}


