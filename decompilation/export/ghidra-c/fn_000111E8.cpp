// Ghidra headless export
// Entry: 0x000111E8
// Name: convert_menu_routine_0x000111e8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00011352 convert_menu_routine_0x00011352
//   0x00011928 render_menu_row
//   0x00011ABE prepare_menu_dialog
//   0x00012C36 prepare_menu_grid_variant_b
//   0x00012EAE convert_menu_routine_0x00012eae


void convert_menu_routine_0x000111e8(void)

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
  prepare_menu_grid_variant_b();
  render_encoded_title_text();
  render_encoded_title_text();
  render_encoded_title_text();
  if (*(char *)(DAT_00fffc04 + 0x57) == '\0') {
    if ((*(ushort *)(DAT_00fffc04 + 0x90) & 8) == 0) {
      if ((*(ushort *)(DAT_00fffc04 + 0x90) & 4) == 0) {
        render_encoded_title_text();
        iVar1 = extraout_A0_01;
      }
      else {
        render_encoded_title_text();
        iVar1 = extraout_A0_00;
      }
    }
    else {
      render_encoded_title_text();
      iVar1 = extraout_A0;
    }
LAB_00011304:
    if (-1 < (char)(*(char *)(iVar1 + 0x57) + -0x10)) goto LAB_00011312;
  }
  else {
    if (-1 < *(char *)(DAT_00fffc04 + 0x56)) {
      render_encoded_title_text();
      iVar1 = extraout_A0_02;
      goto LAB_00011304;
    }
    render_encoded_title_text();
  }
  convert_menu_routine_0x00012eae();
LAB_00011312:
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  convert_menu_routine_0x00011352();
  prepare_menu_dialog();
  return;
}


