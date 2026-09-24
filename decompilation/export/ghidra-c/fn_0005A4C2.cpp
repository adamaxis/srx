// Ghidra headless export
// Entry: 0x0005A4C2
// Name: advance_title_intro_dual_sprites
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378


void advance_title_intro_dual_sprites(void)

{
  ushort uVar1;
  
  if (DAT_00fff146 < 4) {
    if (DAT_00fff148 <= DAT_00ffdf88) {
      DAT_00fff13e = *(undefined4 *)((int)&DAT_0005a562 + (int)(short)(DAT_00fff146 * 4));
      uVar1 = DAT_00fff146 * 4 + 4;
      if (0xf < uVar1) {
        uVar1 = 0;
      }
      DAT_00fff142 = *(undefined4 *)((int)&DAT_0005a562 + (int)(short)uVar1);
      DAT_00fff148 = DAT_00ffdf88 + 6;
      DAT_00fff146 = DAT_00fff146 + 1;
      if (3 < DAT_00fff146) {
        DAT_00fff146 = 0;
      }
    }
    FUN_00001378();
    FUN_00001378();
  }
  return;
}


