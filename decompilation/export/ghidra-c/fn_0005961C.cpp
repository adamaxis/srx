// Ghidra headless export
// Entry: 0x0005961C
// Name: FUN_0005961c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000014CE extract_even_bytes
//   0x00059786 FUN_00059786


void FUN_0005961c(void)

{
  int unaff_D6;
  int *extraout_A1;
  
  extract_even_bytes();
  if ((((DAT_00ffb744 == 'G') && (DAT_00ffb745 == 'A')) && (DAT_00ffb746 == 'M')) &&
     (DAT_00ffb747 == 'E')) {
    FUN_00059786();
    if (*extraout_A1 == unaff_D6) {
      return;
    }
  }
  return;
}


