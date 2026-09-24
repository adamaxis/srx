// Ghidra headless export
// Entry: 0x0005A628
// Name: advance_title_intro_targeted_sweep
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void advance_title_intro_targeted_sweep(void)

{
  int iVar1;
  
  if (DAT_00ffdf88 < DAT_00fff15c) {
    if (DAT_00fff15e < DAT_00fff162) {
      return;
    }
  }
  else {
    if (6 < DAT_00fff15a) {
      return;
    }
    iVar1 = (int)(short)(DAT_00fff15a * 10);
    DAT_00fff156 = *(undefined4 *)((int)&PTR_DAT_0005a6c8 + iVar1);
    DAT_00fff160 = *(undefined2 *)(&DAT_0005a6cc + iVar1);
    DAT_00fff162 = *(ushort *)(&DAT_0005a6ce + iVar1);
    DAT_00fff15c = DAT_00ffdf88 + *(short *)(&DAT_0005a6d0 + iVar1);
    DAT_00fff15e = 0x200;
    DAT_00fff15a = DAT_00fff15a + 1;
  }
  DAT_00fff15e = DAT_00fff15e - 9;
  build_title_sprite_records();
  return;
}


