// Ghidra headless export
// Entry: 0x00058A2E
// Name: FUN_00058a2e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x00011190 FUN_00011190


void FUN_00058a2e(void)

{
  short sVar1;
  char *in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int iVar2;
  
  sVar1 = 7;
  do {
    if ((*in_A0 < '\0') || (*in_A0 != 'E')) {
      FUN_00011190();
      iVar2 = extraout_A0_00;
    }
    else {
      FUN_00001232();
      iVar2 = extraout_A0;
    }
    in_A0 = (char *)(iVar2 + 3);
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


