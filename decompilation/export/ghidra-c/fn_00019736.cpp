// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00019736


void FUN_00019736(void)

{
  if ((-1 < DAT_00fff10c) && ((&DAT_00fff0ed)[(short)(ushort)DAT_00fff0d4] != '\x06')) {
    return;
  }
  (&DAT_00fff0ed)[(short)(ushort)DAT_00fff0d4] = 0;
  FUN_00001232();
  return;
}


