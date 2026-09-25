// Ghidra headless export
// Entry: 0x00058D32
// Name: convert_intro_routine_0x00058d32
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00057C7A convert_intro_routine_0x00057c7a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_intro_routine_0x00058d32(void)

{
  DAT_00ffdd02 = 1;
  build_title_sprite_records();
  _DAT_00ffd980 = 1;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  convert_intro_routine_0x00057c7a();
  return;
}


