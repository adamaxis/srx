// Ghidra headless export
// Entry: 0x0001BB6A
// Name: FUN_0001bb6a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D44 advance_source_prng
//   0x00001378 build_title_sprite_records
//   0x0001A09C FUN_0001a09c
//   0x0001B680 FUN_0001b680


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001bb6a(void)

{
  short sVar1;
  
  if ((DAT_00fff0a1 & 4) == 0) {
    DAT_00ffe0ca = DAT_00ffe0ca - 1;
    if ((short)DAT_00ffe0ca < 0) {
      DAT_00fff0a1 = DAT_00fff0a1 | 4;
      DAT_00ffe0ca = 0x3c;
      return;
    }
    DAT_00ffe0ce = advance_source_prng();
  }
  else {
    DAT_00ffe0ca = DAT_00ffe0ca - 1;
    if ((short)DAT_00ffe0ca < 0) {
      DAT_00fff0a1 = DAT_00fff0a1 & 0xfa;
      FUN_0001b680();
      _DAT_00ffe0d6 = 0;
      _DAT_00ffe0f4 = 1;
      FUN_0001a09c();
      return;
    }
    if (3 < (DAT_00ffe0ca & 7)) {
      build_title_sprite_records();
      build_title_sprite_records();
    }
  }
  sVar1 = 6;
  do {
    build_title_sprite_records();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


