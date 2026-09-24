// Ghidra headless export
// Entry: 0x00017E78
// Name: FUN_00017e78
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


int FUN_00017e78(void)

{
  ushort uVar1;
  
  uVar1 = (ushort)(byte)(&DAT_00ff1110)[(short)(DAT_00fff118 * 0x14)];
  if (-1 < DAT_00fff0a0) {
    if (DAT_00fff0a0 == '\0') {
      uVar1 = uVar1 + 1;
    }
    else {
      uVar1 = uVar1 + 2;
    }
  }
  return (uint)(ushort)(0x1b - uVar1) * 0x14;
}


