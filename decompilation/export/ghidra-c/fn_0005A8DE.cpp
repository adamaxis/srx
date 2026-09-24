// Ghidra headless export
// Entry: 0x0005A8DE
// Name: FUN_0005a8de
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378


void FUN_0005a8de(void)

{
  if (DAT_00fff182 <= DAT_00ffdf88) {
    if ((ushort)(DAT_00ffdf88 - DAT_00fff11e) < 6) {
      return;
    }
    if (2 < DAT_00fff180) {
      return;
    }
    DAT_00fff17c = *(undefined4 *)((int)&PTR_DAT_0005a958 + (int)(short)(DAT_00fff180 * 6));
    DAT_00fff182 = DAT_00ffdf88 +
                   *(short *)((int)&PTR_DAT_0005a95c + (int)(short)(DAT_00fff180 * 6));
    DAT_00fff180 = DAT_00fff180 + 1;
  }
  DAT_00fff184 = DAT_00fff184 + 4;
  FUN_00001378();
  return;
}


