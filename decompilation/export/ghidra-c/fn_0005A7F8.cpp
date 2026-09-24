// Ghidra headless export
// Entry: 0x0005A7F8
// Name: advance_title_intro_linear_sweep_b
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void advance_title_intro_linear_sweep_b(void)

{
  int iVar1;
  
  if (DAT_00ffdf88 < DAT_00fff176) {
    if (DAT_00fff178 < 0x40) {
      return;
    }
  }
  else {
    if (7 < DAT_00fff174) {
      return;
    }
    iVar1 = (int)(short)(DAT_00fff174 << 3);
    DAT_00fff170 = *(undefined4 *)((int)&PTR_DAT_0005a888 + iVar1);
    DAT_00fff17a = *(undefined2 *)(&DAT_0005a88c + iVar1);
    DAT_00fff176 = DAT_00ffdf88 + *(short *)(&DAT_0005a88e + iVar1);
    DAT_00fff178 = 0x200;
    DAT_00fff174 = DAT_00fff174 + 1;
  }
  DAT_00fff178 = DAT_00fff178 - 0xb;
  build_title_sprite_records();
  return;
}


