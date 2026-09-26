// Ghidra branch-target split
// Entry: 0x000577BA
// Name: apply_field_quantity_purchase
// Decompile status: recovered from the verified 0x56DC4 branch target
// Calls recovered from direct call instructions:
//   0x00000D94 wait_for_controller_event
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00057B8C run_menu_confirmation_prompt
//   0x00057C46 write_field_action_grid_variant_b

/*
 * Ghidra's original function inventory folded this routine into the preceding
 * 0x5766A range. The ROM has an explicit branch from 0x56DC4 to 0x577BA and
 * an independent RTS at 0x5787A, so this file records the recovered boundary
 * while retaining the original source-shaped evidence.
 */

void apply_field_quantity_purchase(void)
{
  if (-1 < (char)(DAT_00fff0d0 - 5)) {
    DAT_00fff112 = 0xff;
    return;
  }
  if (DAT_00fff0b3 < 0) {
    DAT_00fff0b3 = 0;
    return;
  }
  write_field_action_grid_variant_b();
  render_encoded_title_text();
  build_grouped_decimal_glyphs();
  build_grouped_decimal_glyphs();
  if (DAT_00fffbfe < DAT_00fff0be) {
    render_encoded_title_text();
    wait_for_controller_event();
  }
  else {
    run_menu_confirmation_prompt();
    if (DAT_00fff0dc == 0) {
      DAT_00fffbfe = DAT_00fffbfe - DAT_00fff0be;
      if (DAT_00fff0d0 == 0) {
        DAT_00fffbbe = DAT_00fff0d6;
      }
      else {
        (&DAT_00fffbbe)[(short)(ushort)(DAT_00fff0d0 + 7)] = DAT_00fff0d6;
      }
    }
  }
  DAT_00fff0b3 = 0;
}
