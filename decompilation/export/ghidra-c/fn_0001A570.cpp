// Ghidra headless export
// Entry: 0x0001A570
// Name: build_title_sprites_twice
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//
// Direct ROM trace:
//   A2=0xF1B1E, D1=0x012A, D2=0x0100, D6=0x2000, then 0x1378
//   A2=0xF1B28, D1=0x0115, D2=0x0100, D6=0x2000, then 0x1378 (tail call)
// This is not merely a repeated callback: the two source lists are the two
// fixed title sprite groups emitted on the negative 0x1A5A0 transition.
// The typed host request is
// ghidra_conversion::TitleSpriteBatchBuildRequest.


void build_title_sprites_twice(void)

{
  build_title_sprite_records();
  build_title_sprite_records();
  return;
}


