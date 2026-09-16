// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00019788


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00019788(void)

{
  short sVar1;
  char *pcVar2;
  
  DAT_00fff0ac = DAT_00fff0ac - 1;
  if ((short)((ushort)DAT_00fff0ac << 8) < 0) {
    DAT_00fff0ac = 0x28;
    FUN_0001792e();
  }
  FUN_00017eee();
  if (DAT_00fff0b8 != '\0') {
    pcVar2 = &DAT_00fff0ed;
    _DAT_00ffdd48 = _DAT_00ffdd48 + -1;
    if (_DAT_00ffdd48 < 0) {
      DAT_00fff108 = 0;
      sVar1 = 4;
      do {
        if (*pcVar2 != '\x06') {
          *pcVar2 = *pcVar2 + '\x01';
          FUN_00001232();
          _DAT_00ffdd48 = *(short *)(&DAT_00fff0f2 + (short)((ushort)DAT_00fff108 << 1));
          if (*pcVar2 != '\x06') {
            return;
          }
          FUN_00017c84();
          FUN_00017c52();
          return;
        }
        pcVar2 = pcVar2 + 1;
        DAT_00fff108 = DAT_00fff108 + 1;
        sVar1 = sVar1 + -1;
      } while (sVar1 != -1);
    }
  }
  return;
}


