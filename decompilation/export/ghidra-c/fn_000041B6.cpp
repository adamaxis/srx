// Ghidra headless export
// Entry: 0x000041B6
// Name: FUN_000041b6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


ushort FUN_000041b6(void)

{
  ushort uVar1;
  
  uVar1 = DAT_00ffddc6 >> 4;
  if (-1 < (short)(uVar1 - 8)) {
    uVar1 = uVar1 - 8;
  }
  return uVar1;
}


