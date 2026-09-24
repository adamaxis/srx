// Ghidra headless export
// Entry: 0x0001CB20
// Name: FUN_0001cb20
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001CA00 FUN_0001ca00
//   0x0001CB6A FUN_0001cb6a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001cb20(void)

{
  int in_A0;
  int extraout_A1;
  
  if ((DAT_00ff1592 & 0x40) != 0) {
    if (DAT_00ff15cc != '\0') {
      return;
    }
    DAT_00ff15cc = 0xff;
    DAT_00ff15d0 = 0;
    DAT_00ff15cf = 0;
    DAT_00ff15cb = 0;
    DAT_00ff15ca = 0x14;
    DAT_00ff15d1 = 0x3c;
    DAT_00ff15c6 = 0x1400;
    DAT_00ff15c8 = 0x1070;
    DAT_00ff15d2 = 0xfff8;
    DAT_00ff15d4 = 0xfff2;
    return;
  }
  if (_DAT_00ffe010 == 0) {
    if (DAT_00fff0b7 < '\0') {
      DAT_00ffe060 = 0x134;
      *(undefined **)(in_A0 + 4) = &UNK_0001c27c;
      *(undefined1 *)(in_A0 + 0xd) = 0;
      FUN_000530de();
      DAT_00ffe062 = 0x102;
      _DAT_00ffe05e = 0xffff;
      DAT_00ffe064 = 0;
      DAT_00ffe066 = 3;
      return;
    }
    if (DAT_00fff0b7 != '\0') {
      FUN_0001ca00();
      return;
    }
  }
  FUN_0001cb6a();
  if (_DAT_00ffe0e2 == 0) {
    return;
  }
  *(short *)(extraout_A1 + 0xe) = *(short *)(extraout_A1 + 0xe) + -6;
  return;
}


