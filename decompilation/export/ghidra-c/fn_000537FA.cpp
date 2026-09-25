// Ghidra headless export
// Entry: 0x000537FA
// Name: convert_audio_routine_0x000537fa
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00053248 build_title_sprite_once


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_audio_routine_0x000537fa(void)

{
  undefined4 *in_A1;
  int unaff_A3;
  
  build_title_sprite_once();
  while (*(char *)((int)in_A1 + (short)(DAT_00fff118 * 0x14) + 7) < '\0') {
    in_A1 = &DAT_00ff1100;
  }
  build_title_sprite_records();
  *(undefined1 *)(unaff_A3 + -5) = 0;
  _DAT_00ffd980 = 1;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  return;
}


