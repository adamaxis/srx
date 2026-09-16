// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000194AC


void FUN_000194ac(void)

{
  char unaff_D5b;
  
  FUN_00019504();
  DAT_00fff0d4 = unaff_D5b + DAT_00fff0d4;
  if ((short)((ushort)DAT_00fff0d4 << 8) < 0) {
    DAT_00fff0d4 = 4;
  }
  else if (-1 < (char)(DAT_00fff0d4 - 5)) {
    DAT_00fff0d4 = 0;
  }
  FUN_00001232();
  FUN_0001792e();
  return;
}


