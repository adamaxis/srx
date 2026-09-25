// Ghidra headless export
// Entry: 0x0001CEFA
// Name: advance_title_effect_position
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001CF36 build_title_sprites


void advance_title_effect_position(void)

{
  DAT_00ffe088 = DAT_00ffe088 + 9;
  DAT_00ffe08a = DAT_00ffe08a + -0x1a;
  DAT_00ffe08c = DAT_00ffe08c + -0xe;
  DAT_00ffe08e = DAT_00ffe08e + -0xe;
  build_title_sprites();
  build_title_sprite_records();
  return;
}


