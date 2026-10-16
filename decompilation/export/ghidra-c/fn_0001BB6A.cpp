// Ghidra headless export
// Entry: 0x0001BB6A
// Name: advance_title_noise_effect
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D44 advance_source_prng
//   0x00001378 build_title_sprite_records
//   0x0001A09C refresh_menu_selection_after_clear
//   0x0001B680 attach_behavior_descriptor_e
// Inline sprite-record streams consumed directly by 0x00001378:
//   0x001DAB8, 0x001DAC2, 0x001DACC, 0x001DAD6
//   0x001DAE0, 0x001DAEA
// The common row starts at x=0xD8, adds 0x10 before each of seven builds,
// uses y=0xEC and D6=0x2000, and rotates a local D4 copy of FFE0CE.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_title_noise_effect(void)

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
      attach_behavior_descriptor_e();
      _DAT_00ffe0d6 = 0;
      _DAT_00ffe0f4 = 1;
      refresh_menu_selection_after_clear();
      return;
    }
    if (3 < (DAT_00ffe0ca & 7)) {
      // Paired top sprites: 0x1DAE0 at (0x158,0xFC), then 0x1DAEA
      // at (0xE7,0xFC), both with D6=0x2000. The common seven-sprite
      // row below still runs after these two builds.
      build_title_sprite_records();
      build_title_sprite_records();
    }
  }
  // A negative inactive countdown returns above without entering this row.
  // Otherwise D4 is a working copy of FFE0CE; the source table selected by
  // its rotations is passed directly to the builder, with no +8 adjustment.
  sVar1 = 6;
  do {
    build_title_sprite_records();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}

