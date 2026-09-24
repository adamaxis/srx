// Ghidra headless export
// Entry: 0x0005AC7E
// Name: advance_title_intro_stage_timed_sprites
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378


void advance_title_intro_stage_timed_sprites(void)

{
  int iVar1;
  
  if (0x3b < (ushort)(DAT_00ffdf88 - DAT_00fff11e)) {
    if ((DAT_00fff1a6 <= DAT_00ffdf88) && (DAT_00fff1a4 < 3)) {
      iVar1 = (int)(short)(DAT_00fff1a4 << 3);
      DAT_00fff1a0 = *(undefined4 *)((int)&PTR_DAT_0005ad02 + iVar1);
      DAT_00fff1a8 = *(undefined2 *)(&DAT_0005ad06 + iVar1);
      DAT_00fff1aa = *(undefined2 *)(&DAT_0005ad08 + iVar1);
      DAT_00fff1a6 = DAT_00ffdf88 + 6;
      DAT_00fff1a4 = DAT_00fff1a4 + 1;
    }
    FUN_00001378();
  }
  return;
}


