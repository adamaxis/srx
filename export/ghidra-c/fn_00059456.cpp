// Ghidra headless export
// Entry: 0x00059456
// Name: normalize_sram_save_slots
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001502 copy_even_bytes
//   0x0005961C validate_sram_game_save


void normalize_sram_save_slots(void)

{
  bool bVar1;
  
  bVar1 = false;
  validate_sram_game_save();
  if (bVar1) {
    bVar1 = false;
    validate_sram_game_save();
    if (bVar1) {
      return;
    }
    copy_even_bytes();
  }
  else {
    bVar1 = false;
    validate_sram_game_save();
    if (bVar1) {
      copy_even_bytes();
    }
  }
  return;
}


