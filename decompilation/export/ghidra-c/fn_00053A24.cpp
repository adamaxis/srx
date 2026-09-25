// Ghidra headless export
// Entry: 0x00053A24
// Name: begin_title_audio_sequence
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00053248 build_title_sprite_once


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void begin_title_audio_sequence(void)

{
  int unaff_A3;
  
  build_title_sprite_once();
  build_title_sprite_records();
  *(undefined1 *)(unaff_A3 + -5) = 0;
  _DAT_00ffd980 = 1;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  return;
}


