// Ghidra headless export
// Entry: 0x0001F292
// Name: FUN_0001f292
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001530 FUN_00001530
//   0x00001728 FUN_00001728
//   0x0001F32E FUN_0001f32e
//   0x0001F34A FUN_0001f34a
//   0x0001F38A FUN_0001f38a
//   0x000566E2 FUN_000566e2
//   0x000566F6 FUN_000566f6


void FUN_0001f292(void)

{
  if (DAT_00fff0b5 != '\0') {
    FUN_0001f32e();
  }
  FUN_000566e2();
  FUN_000566f6();
  FUN_00001530();
  if (DAT_00fff0af < '\0') {
    if (DAT_00fff0ae < '\0') {
      DAT_00fff0ae = '\0';
    }
    FUN_0001f38a();
    FUN_0001f34a();
    DAT_00fff0af = '\0';
  }
  else {
    FUN_00001728();
  }
  return;
}


