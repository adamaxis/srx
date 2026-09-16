// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001C1F6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001c1f6(void)

{
  int extraout_A0;
  
  _DAT_00ffe0ec = _DAT_00ffe0ec + -1;
  if (_DAT_00ffe0ec < 0) {
    _DAT_00ffe0ec = _DAT_00ffe0ea;
    DAT_00ffe169 = DAT_00ffe169 + -1;
    if (DAT_00ffe169 == '\0') {
      FUN_00053112();
      *(byte *)(extraout_A0 + 0x12) = *(byte *)(extraout_A0 + 0x12) & 0xfd;
      FUN_00019f7e();
    }
  }
  return;
}


