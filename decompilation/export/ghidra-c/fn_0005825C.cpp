// Ghidra headless export
// Entry: 0x0005825C
// Name: convert_intro_routine_0x0005825c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00057A6E apply_field_action_cost_discounts
//   0x00057B42 apply_large_field_action_cost_discount
//   0x00057C3A convert_intro_routine_0x00057c3a


void convert_intro_routine_0x0005825c(void)

{
  char *extraout_A0;
  
  render_encoded_title_text();
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  if (-1 < (char)(DAT_00fffbcb - *extraout_A0)) {
    convert_intro_routine_0x00057c3a();
    return;
  }
  if (*(int *)(extraout_A0 + 2) + -0x10000 < 0) {
    apply_field_action_cost_discounts();
  }
  else {
    apply_large_field_action_cost_discount();
  }
  build_grouped_decimal_glyphs();
  return;
}

