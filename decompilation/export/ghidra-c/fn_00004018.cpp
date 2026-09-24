// Ghidra headless export
// Entry: 0x00004018
// Name: FUN_00004018
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003D4A FUN_00003d4a
//   0x00004000 FUN_00004000
//   0x00004216 FUN_00004216


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00004018(void)

{
  short in_D0w;
  
  if (in_D0w < 0) {
    if (-1 < (short)((DAT_00ffddc8 >> 4) - 8)) {
      FUN_00004216();
      FUN_00004000();
      FUN_00003d4a();
      _DAT_00ffdf58 = 0x8000;
      FUN_00003d4a();
      _DAT_00ffdf58 = 0;
    }
  }
  else if ((short)((DAT_00ffddc8 >> 4) + 8) <= DAT_00ffddd0) {
    FUN_00004216();
    FUN_00004000();
    FUN_00003d4a();
    _DAT_00ffdf58 = 0x8000;
    FUN_00003d4a();
    _DAT_00ffdf58 = 0;
  }
  return;
}


