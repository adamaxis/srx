// Ghidra headless export
// Entry: 0x0005322C
// Name: prepare_audio_title
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00053248 build_title_sprite_once


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void prepare_audio_title(void)

{
  int unaff_A3;
  
  build_title_sprite_once();
  *(undefined1 *)(unaff_A3 + -5) = 0;
  _DAT_00ffd980 = 1;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  return;
}


