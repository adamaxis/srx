// Ghidra headless export
// Entry: 0x0005D746
// Name: FUN_0005d746
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 FUN_00000bd2
//   0x0000131C FUN_0000131c
//   0x0000195C FUN_0000195c
//   0x000530D2 FUN_000530d2
//   0x000530DE FUN_000530de
//   0x0005D886 FUN_0005d886
//   0x0005D8BA FUN_0005d8ba
//   0x0005D938 FUN_0005d938
//   0x0005DA50 FUN_0005da50
//   0x0005DAC4 FUN_0005dac4


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005d746(void)

{
  short sVar1;
  
  _DAT_00fffc0a = 1;
  FUN_0005da50();
  DAT_00ffe18a = 1;
  DAT_00ffdd2a = 0x41;
  _DAT_00ffdd2c = 0x119;
  DAT_00ffdd2e = 0x1a8;
  DAT_00ffdd3c = 0;
  sVar1 = 0x150;
  DAT_00ffdf92 = 9;
  do {
    FUN_00000bd2();
    if (DAT_00ffdd3c != 0) {
      FUN_0005d8ba();
    }
    FUN_0005dac4();
    FUN_0005d938();
    if (DAT_00ffdd2e == 0x160) {
LAB_0005d7c8:
      DAT_00ffdd30 = DAT_00ffdd30 + -1;
      if ((DAT_00ffdd30 < 0) && (DAT_00ffdd28 = DAT_00ffdd28 + 1, (DAT_00ffdd28 & 1) == 0)) {
        if (DAT_00ffdd2a < 6) {
          FUN_000530d2();
        }
        DAT_00ffdd2a = DAT_00ffdd2a + -1;
        if (-1 < DAT_00ffdd2a) {
          if (DAT_00ffdd2a == 0) {
            FUN_000530d2();
            DAT_00ffdd3c = 1;
          }
          DAT_00ffdd0a = DAT_00ffdd0a + -1;
          if (4 < DAT_00ffdd2a) {
            FUN_000530de();
          }
        }
      }
    }
    else {
      DAT_00ffdd2e = DAT_00ffdd2e + -1;
      if (DAT_00ffdd2e == 0x160) {
        DAT_00ffdd30 = 0x14;
        goto LAB_0005d7c8;
      }
    }
    DAT_00ffef14 = DAT_00ffef14 | 0xe;
    sVar1 = sVar1 + -1;
    if (sVar1 == -1) {
      FUN_0000195c();
      FUN_0000131c();
      FUN_0005d886();
      sVar1 = 0x78;
      do {
        FUN_00000bd2();
        sVar1 = sVar1 + -1;
      } while (sVar1 != -1);
      FUN_0000195c();
      FUN_0000131c();
      return;
    }
  } while( true );
}


