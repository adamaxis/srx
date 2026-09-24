// Ghidra headless export
// Entry: 0x00003032
// Name: FUN_00003032
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00003032(void)

{
  short in_D0w;
  
  if ((((in_D0w != 6) && (in_D0w != 7)) && (in_D0w != 8)) &&
     ((in_D0w != 9 && ((in_D0w < 0x2c || (0x31 < in_D0w)))))) {
    _DAT_00fffc0e = 0;
    return;
  }
  _DAT_00fffc0e = 1;
  return;
}


