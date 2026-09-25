// Ghidra headless export
// Entry: 0x000563CC
// Name: FUN_000563cc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA count_thresholded_random_rolls
//   0x00019BF6 pass_through_value
//   0x0001B1FC reset_title_transition


void FUN_000563cc(void)

{
  char cVar1;
  char *extraout_A0;
  
  pass_through_value();
  count_thresholded_random_rolls();
  if ((DAT_00fff0dd != '\0') &&
     (cVar1 = DAT_00fff0dd - extraout_A0[1], cVar1 != '\0' && extraout_A0[1] <= DAT_00fff0dd)) {
    cVar1 = *extraout_A0 - cVar1;
    *extraout_A0 = cVar1;
    if (cVar1 < '\0') {
      *extraout_A0 = '\0';
      extraout_A0[7] = '\0';
    }
    if (extraout_A0[7] != '\0') {
      extraout_A0[7] = extraout_A0[7] + -1;
    }
    reset_title_transition();
  }
  return;
}


