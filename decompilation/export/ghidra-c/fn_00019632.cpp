// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00019632


void FUN_00019632(void)

{
  short unaff_D4w;
  
  if ((&DAT_00fffbd8)[unaff_D4w] == '\x03') {
    DAT_00ff1592 = DAT_00ff1592 & 0xdf;
    DAT_00fff0d9 = 0;
  }
  else if ((&DAT_00fffbd8)[unaff_D4w] == '\x05') {
    DAT_00fff0d2 = 0;
  }
  else if ((&DAT_00fffbd8)[unaff_D4w] == '\x06') {
    DAT_00ff1592 = DAT_00ff1592 & 0xfb;
  }
  else if ((&DAT_00fffbd8)[unaff_D4w] == '\a') {
    DAT_00ff1592 = DAT_00ff1592 & 0xfd;
    DAT_00ffe169 = 0;
  }
  (&DAT_00fffbd8)[unaff_D4w] = 0xff;
  (&DAT_00fff0ed)[unaff_D4w] = 6;
  FUN_000011c0();
  FUN_00001232();
  FUN_0001792e();
  return;
}


