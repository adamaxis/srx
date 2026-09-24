// Ghidra headless export
// Entry: 0x0000C802
// Name: FUN_0000c802
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003404 FUN_00003404
//   0x000038FA FUN_000038fa
//   0x0000391A FUN_0000391a
//   0x0000C8B0 FUN_0000c8b0
//   0x0001E514 FUN_0001e514
//   0x00059798 FUN_00059798


void FUN_0000c802(void)

{
  int extraout_A0;
  int extraout_A0_00;
  
  FUN_00059798();
  if (((DAT_00ff0210 & 1) != 0) && ((DAT_00ff024c == '\0' || (DAT_00ff029c == '\0')))) {
    DAT_00fffc0c = 0xff;
    FUN_0000c8b0();
    FUN_0001e514();
    if (extraout_A0 == DAT_00fffc04) {
      FUN_0000391a();
    }
  }
  if (((DAT_00ff0310 & 1) != 0) && ((DAT_00ff034c == '\0' || (DAT_00ff039c == '\0')))) {
    DAT_00fffc0d = 0xff;
    FUN_0000c8b0();
    FUN_0001e514();
    if (extraout_A0_00 == DAT_00fffc04) {
      FUN_0000391a();
    }
  }
  FUN_00003404();
  FUN_000038fa();
  return;
}


