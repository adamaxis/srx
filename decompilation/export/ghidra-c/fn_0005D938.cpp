// Ghidra headless export
// Entry: 0x0005D938
// Name: convert_intro_routine_0x0005d938
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x0005D996 convert_intro_routine_0x0005d996


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_intro_routine_0x0005d938(void)

{
  DAT_00ffdd02 = 1;
  build_title_sprite_records();
  build_title_sprite_records();
  convert_intro_routine_0x0005d996();
  _DAT_00ffd980 = 1;
  return;
}


