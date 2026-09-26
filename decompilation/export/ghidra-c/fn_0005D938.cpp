// Ghidra headless export
// Entry: 0x0005D938
// Name: advance_pre_title_animation
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x0005D996 advance_pre_title_animation_phase


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_pre_title_animation(void)

{
  DAT_00ffdd02 = 1;
  build_title_sprite_records();
  build_title_sprite_records();
  advance_pre_title_animation_phase();
  _DAT_00ffd980 = 1;
  return;
}


