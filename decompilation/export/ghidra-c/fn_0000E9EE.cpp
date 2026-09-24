// Ghidra headless export
// Entry: 0x0000E9EE
// Name: FUN_0000e9ee
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0000e9ee(void)

{
  undefined4 uVar1;
  short in_D0w;
  short unaff_D5w;
  short unaff_D6w;
  undefined4 *puVar2;
  
  puVar2 = (undefined4 *)((short)(in_D0w * 10) + DAT_00ffddec);
  uVar1 = *puVar2;
  if (((((short)((short)((uint)uVar1 >> 0x10) + -0xc) <= unaff_D5w) &&
       (unaff_D5w <= (short)((short)uVar1 + 0xc))) &&
      (uVar1 = puVar2[1], (short)((short)((uint)uVar1 >> 0x10) + -7) <= unaff_D6w)) &&
     (unaff_D6w <= (short)((short)uVar1 + 7))) {
    return;
  }
  return;
}


