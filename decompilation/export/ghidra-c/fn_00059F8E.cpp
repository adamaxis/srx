// Ghidra headless export
// Entry: 0x00059F8E
// Name: FUN_00059f8e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F1C FUN_00000f1c
//   0x00001728 FUN_00001728
//   0x000530DE FUN_000530de
//   0x0005B32C FUN_0005b32c


void FUN_00059f8e(void)

{
  ushort uVar1;
  
  uVar1 = DAT_00ffdf88 - DAT_00fff11e;
  if (uVar1 < 0x169) {
    if (uVar1 == 0x168) {
      FUN_000530de();
      DAT_00ffdd08 = 0;
      FUN_00000f1c();
      FUN_0005b32c();
      FUN_00001728();
    }
    else {
      if (0x11f < uVar1) {
        DAT_00fff124 = DAT_00fff124 + 0xaa;
        DAT_00ffdd08 = DAT_00fff124 >> 8;
        DAT_00ffef14 = DAT_00ffef14 | 4;
      }
      if (DAT_00fff122 <= DAT_00ffdf88) {
        DAT_00fff122 = DAT_00ffdf88 +
                       *(short *)((int)&PTR_DAT_0005a084 + (int)(short)(DAT_00fff120 * 6));
        FUN_00001728();
        DAT_00fff120 = DAT_00fff120 + 1;
        if (5 < DAT_00fff120) {
          DAT_00fff120 = 0;
        }
      }
    }
  }
  return;
}


