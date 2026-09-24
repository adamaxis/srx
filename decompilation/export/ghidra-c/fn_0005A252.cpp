// Ghidra headless export
// Entry: 0x0005A252
// Name: FUN_0005a252
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378
//   0x000530DE FUN_000530de
//   0x0005A582 FUN_0005a582
//   0x0005B2C2 FUN_0005b2c2
//   0x0005B2E2 FUN_0005b2e2
//   0x0005B32C FUN_0005b32c


void FUN_0005a252(void)

{
  if (DAT_00fff12e < 0xc) {
    if (DAT_00fff130 <= DAT_00ffdf88) {
      if (DAT_00fff12e < 4) {
        DAT_00fff12a = *(undefined4 *)((int)&PTR_PTR_0005a398 + (int)(short)(DAT_00fff12e * 6));
        DAT_00fff130 = DAT_00ffdf88 + *(short *)(&DAT_0005a39c + (short)(DAT_00fff12e * 6));
        DAT_00fff12e = DAT_00fff12e + 1;
      }
      else {
        if (DAT_00fff12e == 4) {
          FUN_000530de();
        }
        if (DAT_00fff12e == 4) {
          DAT_00fff146 = 4;
          FUN_0005b2c2();
          DAT_00fff12e = DAT_00fff12e + 1;
          DAT_00fff130 = DAT_00ffdf88 + 9;
        }
        else if (DAT_00fff130 <= DAT_00ffdf88) {
          FUN_0005b2e2();
          DAT_00fff130 = DAT_00ffdf88 + 9;
          DAT_00fff12e = DAT_00fff12e + 1;
          if (DAT_00fff12e == 9) {
            FUN_0005a582();
          }
          if (0xb < DAT_00fff12e) {
            FUN_0005b32c();
          }
        }
      }
    }
    FUN_00001378();
  }
  return;
}


