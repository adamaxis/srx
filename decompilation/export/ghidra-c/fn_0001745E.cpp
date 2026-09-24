// Ghidra headless export
// Entry: 0x0001745E
// Name: FUN_0001745e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E34 FUN_00000e34
//   0x00000FFE FUN_00000ffe
//   0x0000102A FUN_0000102a
//   0x00001530 FUN_00001530
//   0x00001728 FUN_00001728
//   0x0000334C FUN_0000334c
//   0x000033BE FUN_000033be
//   0x00003404 FUN_00003404
//   0x000039F2 FUN_000039f2
//   0x000176BA FUN_000176ba
//   0x000176EA FUN_000176ea
//   0x00017708 FUN_00017708
//   0x000177A4 FUN_000177a4
//   0x000177CC FUN_000177cc
//   0x000177FC FUN_000177fc
//   0x0001789C FUN_0001789c
//   0x00017EB4 FUN_00017eb4
//   0x00019856 FUN_00019856
//   0x0001ADE6 FUN_0001ade6
//   0x0001B5A6 FUN_0001b5a6
//   0x0001B628 FUN_0001b628
//   0x0001C4EE FUN_0001c4ee
//   0x000530D2 FUN_000530d2
//   0x000530DE FUN_000530de
//   0x000530EA FUN_000530ea
//   0x00053126 FUN_00053126
//   0x000531EA FUN_000531ea
//   0x00053290 FUN_00053290
//   0x000533EE FUN_000533ee


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001745e(void)

{
  int extraout_A0;
  
  VDP_CTRL = 0x8134;
  DAT_00ffe189 = DAT_00fff0a0;
  DAT_00fffbca = 10;
  DAT_00fff0a2 = 0;
  DAT_00fff0b1 = '\0';
  DAT_00fff11a = 0xffff;
  if (DAT_00fffbbc != DAT_00fffbbd) {
    FUN_00053290();
    DAT_00fffbbd = DAT_00fffbbc;
  }
  DAT_00ffdd48 = 0;
  FUN_000531ea();
  FUN_00017eb4();
  DAT_00fff0d2 = 0;
  do {
    VDP_CTRL = 0x8134;
    DAT_00fff0e5 = DAT_00fff0a7;
    DAT_00fff0b7 = 0;
    _DAT_00fff0e6 = 0;
    DAT_00ffdf84 = 0;
    DAT_00fff0a6 = 0;
    DAT_00fff0a9 = 0;
    DAT_00fff0ad = 0;
    DAT_00fff0a3 = 1;
    DAT_00fff0e4 = 0xc;
    DAT_00fff0ac = 0x28;
    FUN_000533ee();
    if (DAT_00ffdf40 != 0) {
      DAT_00ffe171 = 0xff;
    }
    FUN_00053126();
    _DAT_00ffe172 = 0xffff;
    _DAT_00ffdf8a = 0xffff;
    DAT_00fff106 = 0xffff;
    DAT_00fff0e1 = 0;
    DAT_00fff0de = 5;
    DAT_00fff0df = 0;
    FUN_00000ffe();
    FUN_0000102a();
    FUN_00000e34();
    VDP_CTRL = 0x8b03;
    FUN_0001b5a6();
    VDP_CTRL = 0x9100;
    FUN_000176ea();
    FUN_000177cc();
    FUN_00001530();
    FUN_00001728();
    FUN_00001728();
    FUN_00017708();
    FUN_000177a4();
    FUN_00019856();
    FUN_000033be();
    FUN_000176ba();
    FUN_0001c4ee();
    DAT_00ff158e = 0x120;
    DAT_00ff1590 = 0x12f;
    FUN_0001b628();
    *(undefined1 *)(extraout_A0 + 2) = 0x1e;
    *(undefined1 *)(extraout_A0 + 3) = 0x1e;
    FUN_0001789c();
    VDP_CTRL = 0x8174;
    FUN_000530de();
    FUN_000177fc();
    FUN_000530d2();
    FUN_000530ea();
    FUN_00001728();
    FUN_0001ade6();
  } while (DAT_00fff0b1 == '\0');
  VDP_CTRL = 0x8134;
  VDP_CTRL = 0x9191;
  FUN_00003404();
  FUN_0000334c();
  FUN_000039f2();
  FUN_00053126();
  return;
}


