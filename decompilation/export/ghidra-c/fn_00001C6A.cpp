// Ghidra headless export
// Entry: 0x00001C6A
// Name: FUN_00001c6a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00001c6a(void)

{
  DAT_00ffdd02 = 1;
  DAT_00ffdd3c = DAT_00ffdd3c + -1;
  if (DAT_00ffdd3c < 0) {
    DAT_00ffdd3c = 3;
    _DAT_00ffdd3e = _DAT_00ffdd3e + 1;
    if (5 < _DAT_00ffdd3e) {
      _DAT_00ffdd3e = 0;
    }
  }
  DAT_00ffdd42 = DAT_00ffdd42 + 4;
  FUN_00001378();
  _DAT_00ffd980 = 1;
  return;
}


