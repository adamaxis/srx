// Ghidra headless export
// Entry: 0x00026972
// Name: FUN_00026972
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00026972(void)

{
  if (DAT_00fff0d2 != '\0') {
    if (_DAT_00ffe0ee != 0) {
      DAT_00ffe0f0 = DAT_00ffe0f0 + -1;
      if (DAT_00ffe0f0 < 0) {
        _DAT_00ffe0ee = 0;
      }
      else if (1 < (DAT_00ffdf88 & 3)) {
        return;
      }
    }
    build_title_sprite_records();
  }
  return;
}


