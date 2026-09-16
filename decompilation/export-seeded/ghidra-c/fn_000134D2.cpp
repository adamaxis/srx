// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000134D2


void FUN_000134d2(void)

{
  DAT_00fff09c = (&DAT_000d4456)[(short)(ushort)DAT_00fffbf8];
  FUN_00001530();
  FUN_00001728();
  FUN_00000bd2();
  FUN_00000bf2();
  FUN_0000195c();
  if (DAT_00fff09e == '\0') {
    FUN_0000e6f4();
  }
  if (DAT_00ffe18b != '\0') {
    DAT_00fff09c = 4;
                    /* WARNING: Subroutine does not return */
    FUN_00013726();
  }
                    /* WARNING: Subroutine does not return */
  FUN_00013726();
}


