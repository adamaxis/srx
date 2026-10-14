// Ghidra headless export
// Entry: 0x0005961C
// Name: validate_sram_game_save
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000014CE extract_even_bytes
//   0x00059786 sum_game_save_payload


void validate_sram_game_save(void)

{
  int unaff_D6;
  int *extraout_A1;
  
  extract_even_bytes();
  if ((((DAT_00ffb744 == 'G') && (DAT_00ffb745 == 'A')) && (DAT_00ffb746 == 'M')) &&
     (DAT_00ffb747 == 'E')) {
    sum_game_save_payload();
    if (*extraout_A1 == unaff_D6) {
      return;
    }
  }
  return;
}


