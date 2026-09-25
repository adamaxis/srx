// Ghidra headless export
// Entry: 0x0001A570
// Name: build_title_sprites_twice
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void build_title_sprites_twice(void)

{
  build_title_sprite_records();
  build_title_sprite_records();
  return;
}


