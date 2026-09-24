// Ghidra headless export
// Entry: 0x00001872
// Name: FUN_00001872
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000EC0 FUN_00000ec0
//   0x00000EE6 FUN_00000ee6
//   0x00000F1C FUN_00000f1c
//   0x00000F3C FUN_00000f3c
//   0x00002C92 FUN_00002c92
//   0x00002CB2 FUN_00002cb2
//   0x00006148 FUN_00006148
//   0x000061F4 FUN_000061f4
//   0x0005672C FUN_0005672c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00001872(void)

{
  byte bVar1;
  
  DAT_00fff9ec = DAT_00fff9ec + 1;
  DAT_00fff9f0 = 0;
  bVar1 = DAT_00ffef14 & 2;
  DAT_00ffef14 = DAT_00ffef14 & 0xfd;
  if (bVar1 != 0) {
    FUN_00000ee6();
    DAT_00ffdd02 = 1;
  }
  bVar1 = DAT_00ffef14 & 1;
  DAT_00ffef14 = DAT_00ffef14 & 0xfe;
  if (bVar1 != 0) {
    FUN_00000ec0();
  }
  if (_DAT_00ffdfe8 == 0) {
    FUN_0005672c();
  }
  bVar1 = DAT_00ffef14 & 4;
  DAT_00ffef14 = DAT_00ffef14 & 0xfb;
  if (bVar1 != 0) {
    FUN_00000f1c();
  }
  bVar1 = DAT_00ffef14 & 8;
  DAT_00ffef14 = DAT_00ffef14 & 0xf7;
  if (bVar1 != 0) {
    FUN_00000f3c();
  }
  if (_DAT_00ffe100 != 0) {
    FUN_00002c92();
  }
  if (_DAT_00ffe102 != 0) {
    FUN_00002cb2();
  }
  if (DAT_00ffdf2a != 0) {
    FUN_00006148();
  }
  if (DAT_00ffdf30 != 0) {
    FUN_000061f4();
  }
  return;
}


