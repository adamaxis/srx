// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001F292


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


