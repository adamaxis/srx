// Ghidra headless export
// Entry: 0x0005766A
// Name: apply_field_resource_purchase
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D94 wait_for_controller_event
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00057B8C run_menu_confirmation_prompt
//   0x00057C46 refresh_field_action_detail_panel


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void apply_field_resource_purchase(void)

{
  int extraout_D1;

  if (-1 < (char)(DAT_00fff0d0 - 4)) {
    DAT_00fff112 = 0xff;
    return;
  }
  refresh_field_action_detail_panel();
  if (DAT_00fff0d0 == 2) {
    if (DAT_00fffbc4 < (char)(DAT_00fffbbe * '\n')) goto LAB_0005770c;
  }
  else if (DAT_00fff0d0 < 3) {
    if (DAT_00fff0d0 == 0) {
      if (DAT_00fffbc0 < (short)((ushort)DAT_00fffbbe * 0x28)) goto LAB_0005770c;
    }
    else if (DAT_00fffbc2 < (short)((ushort)DAT_00fffbbe * 0x50)) {
LAB_0005770c:
      if (DAT_00fffbfe < (int)(uint)*(ushort *)(&DAT_001ac8e0 + (uint)DAT_00fff0d0 * 2)) {
        render_encoded_title_text();
        wait_for_controller_event();
        return;
      }
      _DAT_00fff0be = (uint)*(ushort *)(&DAT_001ac8e0 + (uint)DAT_00fff0d0 * 2);
      render_encoded_title_text();
      build_grouped_decimal_glyphs();
      run_menu_confirmation_prompt();
      if (DAT_00fff0dc == '\0') {
        DAT_00fffbfe = DAT_00fffbfe - extraout_D1;
        if (DAT_00fff0d0 == 0) {
          DAT_00fffbc0 = DAT_00fffbc0 + 10;
          return;
        }
        if ((char)(DAT_00fff0d0 - 2) < '\0') {
          DAT_00fffbc2 = DAT_00fffbc2 + 0x19;
          return;
        }
        if (DAT_00fff0d0 != 2) {
          DAT_00fffbc5 = DAT_00fffbc5 + '\x01';
          return;
        }
        DAT_00fffbc4 = DAT_00fffbc4 + '\x05';
      }
      return;
    }
  }
  else if ((char)(DAT_00fffbc5 + -3) < '\0') goto LAB_0005770c;
  render_encoded_title_text();
  wait_for_controller_event();
  return;
}
