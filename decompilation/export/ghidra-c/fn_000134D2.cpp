// Ghidra headless export
// Entry: 0x000134D2
// Name: FUN_000134d2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 FUN_00000bd2
//   0x00000BF2 FUN_00000bf2
//   0x00001530 FUN_00001530
//   0x00001728 FUN_00001728
//   0x0000195C FUN_0000195c
//   0x0000391A FUN_0000391a
//   0x0000E6F4 FUN_0000e6f4
//   0x00010AF0 FUN_00010af0
//   0x000136DE FUN_000136de
//   0x00013726 FUN_00013726
//   0x00020714 FUN_00020714
//   0x00053126 FUN_00053126


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000134d2(void)

{
  char cVar1;
  short sVar2;
  
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
    FUN_00013726();
    sVar2 = 0x40;
    while( true ) {
      FUN_00000bd2();
      FUN_00000bf2();
      FUN_0000195c();
      if (DAT_00fff09e == '\0') {
        FUN_0000e6f4();
      }
      if (_DAT_00ffddc2 != 0) break;
      sVar2 = sVar2 + -1;
      if (sVar2 == -1) {
        DAT_00fff09c = 2;
        FUN_00013726();
        sVar2 = 0x60;
        do {
          FUN_00000bd2();
          FUN_00000bf2();
          FUN_0000195c();
          if (DAT_00fff09e == '\0') {
            FUN_0000e6f4();
          }
        } while ((_DAT_00ffddc2 == 0) && (sVar2 = sVar2 + -1, sVar2 != -1));
        DAT_00fff09e = 0;
        return;
      }
    }
    DAT_00fff09e = 0;
    return;
  }
  FUN_00013726();
  while( true ) {
    FUN_00000bd2();
    FUN_00000bf2();
    FUN_0000195c();
    if (DAT_00fff09e == '\0') {
      FUN_0000e6f4();
    }
    if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 8) != 0)) {
      DAT_00fff09c = DAT_00fff09c + -1;
      if (DAT_00fff09c < '\0') {
        DAT_00fff09c = '\x06';
      }
      FUN_00013726();
    }
    if (((DAT_00ffdd22._0_1_ & 4) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) {
      cVar1 = DAT_00fff09c + -6;
      DAT_00fff09c = DAT_00fff09c + '\x01';
      if (-1 < cVar1) {
        DAT_00fff09c = '\0';
      }
      FUN_00013726();
    }
    if (((DAT_00ffdd22._0_1_ & 0x40) != 0) || ((DAT_00ffdd22._0_1_ & 0x10) != 0)) break;
    if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
      FUN_0000391a();
    }
    if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
      FUN_00010af0();
LAB_000136b6:
      if ((DAT_00fff0e0 == '\0') && ((char)DAT_00fffbf8 != DAT_00ffddc4)) {
        FUN_000136de();
        FUN_00053126();
      }
      return;
    }
  }
  FUN_00020714();
  DAT_00fff0dc = 0xff;
  goto LAB_000136b6;
}


