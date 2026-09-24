// Ghidra headless export
// Entry: 0x0005766A
// Name: FUN_0005766a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D94 FUN_00000d94
//   0x0000C0DA FUN_0000c0da
//   0x0000C216 FUN_0000c216
//   0x00057B8C FUN_00057b8c
//   0x00057C46 FUN_00057c46


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005766a(void)

{
  int extraout_D1;
  
  if (-1 < (char)(DAT_00fff0d0 - 4)) {
    DAT_00fff112 = 0xff;
    return;
  }
  FUN_00057c46();
  if (DAT_00fff0d0 == 2) {
    if (DAT_00fffbc4 < (char)(DAT_00fffbbe * '\n')) goto LAB_0005770c;
  }
  else if (DAT_00fff0d0 < 3) {
    if (DAT_00fff0d0 == 0) {
      if (DAT_00fffbc0 < (short)((ushort)DAT_00fffbbe * 0x28)) goto LAB_0005770c;
    }
    else if (DAT_00fffbc2 < (short)((ushort)DAT_00fffbbe * 0x50)) {
LAB_0005770c:
      if (DAT_00fffbfe < (int)(uint)*(ushort *)(&DAT_001ac8e0 + (uint)DAT_00fff0d0 * 2)) {
        FUN_0000c0da();
        FUN_00000d94();
        return;
      }
      _DAT_00fff0be = (uint)*(ushort *)(&DAT_001ac8e0 + (uint)DAT_00fff0d0 * 2);
      FUN_0000c0da();
      FUN_0000c216();
      FUN_00057b8c();
      if (DAT_00fff0dc == '\0') {
        DAT_00fffbfe = DAT_00fffbfe - extraout_D1;
        if (DAT_00fff0d0 == 0) {
          DAT_00fffbc0 = DAT_00fffbc0 + 10;
          return;
        }
        if ((char)(DAT_00fff0d0 - 2) < '\0') {
          DAT_00fffbc2 = DAT_00fffbc2 + 0x19;
          return;
        }
        if (DAT_00fff0d0 != 2) {
          DAT_00fffbc5 = DAT_00fffbc5 + '\x01';
          return;
        }
        DAT_00fffbc4 = DAT_00fffbc4 + '\x05';
      }
      return;
    }
  }
  else if ((char)(DAT_00fffbc5 + -3) < '\0') goto LAB_0005770c;
  FUN_0000c0da();
  FUN_00000d94();
  return;
}


