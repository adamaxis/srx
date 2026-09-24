// Ghidra headless export
// Entry: 0x000593FC
// Name: FUN_000593fc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000014CE extract_even_bytes
//   0x00059786 FUN_00059786


void FUN_000593fc(void)

{
  int unaff_D6;
  int *extraout_A1;
  
  extract_even_bytes();
  if ((((DAT_00fff1ec == 'G') && (DAT_00fff1ed == 'A')) && (DAT_00fff1ee == 'M')) &&
     (DAT_00fff1ef == 'E')) {
    FUN_00059786();
    if (*extraout_A1 == unaff_D6) {
      return;
    }
  }
  return;
}


