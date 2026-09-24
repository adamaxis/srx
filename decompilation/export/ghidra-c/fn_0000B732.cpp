// Ghidra headless export
// Entry: 0x0000B732
// Name: FUN_0000b732
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000B77E FUN_0000b77e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort FUN_0000b732(void)

{
  ushort uVar1;
  ushort uVar2;
  
  uVar2 = FUN_0000b77e();
  if (((short)uVar2 < 0) &&
     (((_DAT_00ffdf84 != 0 || (uVar1 = uVar2 & 0x1f, 0x17 < uVar1)) ||
      (((DAT_00ffddc4 == 0x14 || (DAT_00ffdf40 == 2)) && ((1 < uVar1 && (uVar1 < 6)))))))) {
    return 0x7f;
  }
  return uVar2;
}


