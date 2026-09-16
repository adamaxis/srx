// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00012A54


undefined4 FUN_00012a54(void)

{
  undefined4 in_D0;
  
  if ((short)((short)in_D0 + -0xd) < 0) {
    if (DAT_00fff100 == '\0') {
      return in_D0;
    }
    DAT_00fff100 = 0;
  }
  else {
    if (DAT_00fff100 != '\0') {
      return in_D0;
    }
    DAT_00fff100 = 1;
  }
  FUN_0001107e();
  return in_D0;
}


