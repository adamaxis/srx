// Ghidra headless export
// Entry: 0x00055786
// Name: FUN_00055786
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA count_thresholded_random_rolls
//   0x0001047E FUN_0001047e
//   0x00055828 FUN_00055828


void FUN_00055786(void)

{
  int in_A0;
  int extraout_A0;
  int iVar1;
  int extraout_A0_00;
  
  if ((*(byte *)(in_A0 + 0xdf) & 1) == 0) {
    if ((*(byte *)(in_A0 + 0xdf) & 2) == 0) {
      return;
    }
    FUN_00055828();
    iVar1 = extraout_A0_00;
  }
  else {
    FUN_00055828();
    iVar1 = extraout_A0;
  }
  if ((short)(*(short *)(iVar1 + 0x3e) + -3) < 0) {
    FUN_0001047e();
  }
  count_thresholded_random_rolls();
  return;
}


