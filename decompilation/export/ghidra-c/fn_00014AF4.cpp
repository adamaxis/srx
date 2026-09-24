// Ghidra headless export
// Entry: 0x00014AF4
// Name: FUN_00014af4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014756 FUN_00014756
//   0x00014800 FUN_00014800
//   0x00014902 FUN_00014902


void FUN_00014af4(void)

{
  short sVar1;
  short extraout_D1w;
  short unaff_D5w;
  int extraout_A0;
  
  FUN_00014902();
  sVar1 = 4;
  do {
    FUN_00014800();
    if (extraout_D1w != unaff_D5w) {
      *(undefined2 *)(extraout_A0 + 0xba) = 0;
      *(short *)(extraout_A0 + 0xbc) = extraout_D1w;
      *(undefined2 *)(extraout_A0 + 0xbe) = 0xffff;
      return;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_00014756();
  return;
}


