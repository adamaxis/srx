// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000C802


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000c802(void)

{
  int extraout_A0;
  int extraout_A0_00;
  
  FUN_00059798();
  if (((DAT_00ff0210 & 1) != 0) && ((DAT_00ff024c == '\0' || (DAT_00ff029c == '\0')))) {
    DAT_00fffc0c = 0xff;
    FUN_0000c8b0();
    FUN_0001e514();
    if (extraout_A0 == _DAT_00fffc04) {
      FUN_0000391a();
    }
  }
  if (((DAT_00ff0310 & 1) != 0) && ((DAT_00ff034c == '\0' || (DAT_00ff039c == '\0')))) {
    DAT_00fffc0d = 0xff;
    FUN_0000c8b0();
    FUN_0001e514();
    if (extraout_A0_00 == _DAT_00fffc04) {
      FUN_0000391a();
    }
  }
  FUN_00003404();
  FUN_000038fa();
  return;
}


