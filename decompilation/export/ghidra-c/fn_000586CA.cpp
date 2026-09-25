// Ghidra headless export
// Entry: 0x000586CA
// Name: convert_intro_routine_0x000586ca
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x000130C0 convert_menu_routine_0x000130c0
//   0x00057A44 calculate_field_action_record_cost
//   0x00057A6E apply_field_action_cost_discounts
//   0x0005878E convert_intro_routine_0x0005878e


void convert_intro_routine_0x000586ca(void)

{
  uint in_D0;
  int in_A0;
  char *pcVar1;
  
  pcVar1 = (char *)((uint)DAT_00fff0d0 * 3 + (in_D0 & 0xffff) * 0x18 + in_A0);
  if (-1 < *pcVar1) {
    if (*pcVar1 == 'E') {
      write_vdp_grid();
      render_encoded_title_text();
      apply_field_action_cost_discounts();
      build_grouped_decimal_glyphs();
      render_encoded_title_text();
      return;
    }
    convert_menu_routine_0x000130c0();
    convert_intro_routine_0x0005878e();
    render_encoded_title_text();
    calculate_field_action_record_cost();
    build_grouped_decimal_glyphs();
  }
  render_encoded_title_text();
  return;
}

