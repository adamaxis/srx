// Ghidra headless export
// Entry: 0x0001CF4A
// Name: FUN_0001cf4a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001CF36 build_title_sprites


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001cf4a(void)

{
  DAT_00ffe098 = DAT_00ffe098 + 9;
  DAT_00ffe09c = DAT_00ffe09c + 9;
  DAT_00ffe0a0 = DAT_00ffe0a0 + -0x1a;
  DAT_00ffe0a4 = DAT_00ffe0a4 + -0x1a;
  DAT_00ffe090 = DAT_00ffe090 + -8;
  DAT_00ffe094 = DAT_00ffe094 + -8;
  if (_DAT_00ffe0a8 == 0) {
    DAT_00ffe092 = DAT_00ffe092 + -0x36;
    DAT_00ffe096 = DAT_00ffe096 + 0x1a;
    DAT_00ffe09a = DAT_00ffe09a + -0x36;
    DAT_00ffe09e = DAT_00ffe09e + 0x1a;
    DAT_00ffe0a2 = DAT_00ffe0a2 + -0x36;
    DAT_00ffe0a6 = DAT_00ffe0a6 + 0x1a;
  }
  else {
    DAT_00ffe092 = DAT_00ffe092 + -0x18;
    DAT_00ffe096 = DAT_00ffe096 + -4;
    DAT_00ffe09a = DAT_00ffe09a + -0x18;
    DAT_00ffe09e = DAT_00ffe09e + -4;
    DAT_00ffe0a2 = DAT_00ffe0a2 + -0x18;
    DAT_00ffe0a6 = DAT_00ffe0a6 + -4;
  }
  build_title_sprites();
  build_title_sprites();
  build_title_sprites();
  build_title_sprites();
  build_title_sprites();
  build_title_sprites();
  return;
}


