// Ghidra headless export
// Entry: 0x0005A3BE
// Name: advance_title_intro_motion
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void advance_title_intro_motion(void)

{
  int iVar1;
  
  if (DAT_00fff138 <= DAT_00ffdf88) {
    if (4 < DAT_00fff136) {
      return;
    }
    iVar1 = (int)(short)(DAT_00fff136 * 10);
    DAT_00fff132 = *(undefined4 *)((int)&PTR_DAT_0005a482 + iVar1);
    DAT_00fff138 = DAT_00ffdf88 + *(short *)(&DAT_0005a486 + iVar1);
    if (*(short *)(&DAT_0005a488 + iVar1) != 0) {
      DAT_00fff13a = *(short *)(&DAT_0005a488 + iVar1);
    }
    DAT_00fff13c = *(short *)(&DAT_0005a48a + iVar1);
    DAT_00fff136 = DAT_00fff136 + 1;
    if ((4 < DAT_00fff136) && (DAT_00fff136 = 1, 0x167 < (ushort)(DAT_00ffdf88 - DAT_00fff11e))) {
      DAT_00fff136 = 5;
      return;
    }
  }
  DAT_00fff13a = DAT_00fff13c + DAT_00fff13a;
  build_title_sprite_records();
  return;
}


