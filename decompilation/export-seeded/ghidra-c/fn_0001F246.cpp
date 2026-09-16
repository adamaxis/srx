// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001F246


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001f246(void)

{
  code *in_A1;
  
  _DAT_00c00004 = 0x9100;
  FUN_0000c0c2();
  FUN_00010984();
  FUN_0001f292();
  if (DAT_00fff10c != '\0') {
    (*in_A1)();
  }
  DAT_00fff10c = 0;
  FUN_0001f3c4();
  FUN_00010952();
  FUN_0005670e();
  _DAT_00c00004 = 0x9191;
  return;
}


