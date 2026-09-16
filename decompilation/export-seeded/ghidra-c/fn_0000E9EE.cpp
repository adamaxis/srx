// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000E9EE


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000e9ee(void)

{
  undefined4 uVar1;
  short in_D0w;
  short unaff_D5w;
  short unaff_D6w;
  undefined4 *puVar2;
  
  puVar2 = (undefined4 *)((short)(in_D0w * 10) + _DAT_00ffddec);
  uVar1 = *puVar2;
  if (((((short)((short)((uint)uVar1 >> 0x10) + -0xc) <= unaff_D5w) &&
       (unaff_D5w <= (short)((short)uVar1 + 0xc))) &&
      (uVar1 = puVar2[1], (short)((short)((uint)uVar1 >> 0x10) + -7) <= unaff_D6w)) &&
     (unaff_D6w <= (short)((short)uVar1 + 7))) {
    return;
  }
  return;
}


