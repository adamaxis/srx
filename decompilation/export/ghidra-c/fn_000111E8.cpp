// Ghidra headless export
// Entry: 0x000111E8
// Name: FUN_000111e8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00011352 FUN_00011352
//   0x00011928 render_menu_row
//   0x00011ABE prepare_menu_dialog
//   0x00012C36 FUN_00012c36
//   0x00012EAE FUN_00012eae


void FUN_000111e8(void)

{
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int iVar1;
  
  if (DAT_00fff112 == '\0') {
    render_menu_row();
    return;
  }
  DAT_00fff112 = 0;
  FUN_00012c36();
  render_title_text_source();
  render_title_text_source();
  render_title_text_source();
  if (*(char *)(DAT_00fffc04 + 0x57) == '\0') {
    if ((*(ushort *)(DAT_00fffc04 + 0x90) & 8) == 0) {
      if ((*(ushort *)(DAT_00fffc04 + 0x90) & 4) == 0) {
        render_title_text_source();
        iVar1 = extraout_A0_01;
      }
      else {
        render_title_text_source();
        iVar1 = extraout_A0_00;
      }
    }
    else {
      render_title_text_source();
      iVar1 = extraout_A0;
    }
LAB_00011304:
    if (-1 < (char)(*(char *)(iVar1 + 0x57) + -0x10)) goto LAB_00011312;
  }
  else {
    if (-1 < *(char *)(DAT_00fffc04 + 0x56)) {
      render_title_text_source();
      iVar1 = extraout_A0_02;
      goto LAB_00011304;
    }
    render_title_text_source();
  }
  FUN_00012eae();
LAB_00011312:
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  FUN_00011352();
  prepare_menu_dialog();
  return;
}


