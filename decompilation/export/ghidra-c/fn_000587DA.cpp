// Ghidra headless export
// Entry: 0x000587DA
// Name: FUN_000587da
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x00011190 FUN_00011190
//   0x0005763C FUN_0005763c
//   0x00058A2E FUN_00058a2e
//   0x00058AA4 FUN_00058aa4
//   0x00058B1E FUN_00058b1e


void FUN_000587da(void)

{
  short sVar1;
  short sVar2;
  
  DAT_00fff0b8 = DAT_00fff0d0;
  if (DAT_00fff0b7 < '\0') {
    if (((DAT_00fff0d0 != '\0') && (-1 < (char)(DAT_00fff0d0 + -2))) && (DAT_00fff0d0 != '\x02')) {
      if (DAT_00fff0d0 == '\x03') {
        FUN_00058b1e();
        return;
      }
LAB_0005884c:
      FUN_00058aa4();
      return;
    }
  }
  else {
    if (DAT_00fff0b7 != '\0') {
      if (DAT_00fff0b7 == '\x01') {
        sVar1 = 7;
        do {
          FUN_00001232();
          sVar1 = sVar1 + -1;
        } while (sVar1 != -1);
        return;
      }
      if (-1 < (char)(DAT_00fff0d0 + -3)) {
        if (DAT_00fff0d0 == '\x03') {
          sVar1 = 7;
          do {
            FUN_00001232();
            sVar1 = sVar1 + -1;
          } while (sVar1 != -1);
          return;
        }
        if (DAT_00fff0d0 == '\x05') {
          sVar1 = 4;
          sVar2 = 2;
        }
        else {
          sVar1 = 3;
          sVar2 = 3;
        }
        do {
          FUN_00001232();
          sVar1 = sVar1 + -1;
        } while (sVar1 != -1);
        do {
          FUN_00001232();
          sVar2 = sVar2 + -1;
        } while (sVar2 != -1);
        return;
      }
      FUN_0005763c();
      sVar1 = 7;
      do {
        FUN_00001232();
        sVar1 = sVar1 + -1;
      } while (sVar1 != -1);
      return;
    }
    if (DAT_00fff0d0 != '\x01') {
      if ((char)(DAT_00fff0d0 + -1) < '\0') {
        sVar1 = 7;
        do {
          FUN_00011190();
          sVar1 = sVar1 + -1;
        } while (sVar1 != -1);
        return;
      }
      if (DAT_00fff0d0 == '\x03') goto LAB_0005884c;
    }
  }
  FUN_00058a2e();
  return;
}


