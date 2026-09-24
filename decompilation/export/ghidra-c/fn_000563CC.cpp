// Ghidra headless export
// Entry: 0x000563CC
// Name: FUN_000563cc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA FUN_00000dba
//   0x00019BF6 FUN_00019bf6
//   0x0001B1FC FUN_0001b1fc


void FUN_000563cc(void)

{
  char cVar1;
  char *extraout_A0;
  
  FUN_00019bf6();
  FUN_00000dba();
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
    FUN_0001b1fc();
  }
  return;
}


