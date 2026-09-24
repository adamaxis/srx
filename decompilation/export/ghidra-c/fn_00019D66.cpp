// Ghidra headless export
// Entry: 0x00019D66
// Name: FUN_00019d66
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017998 render_menu_text_progression
//   0x00017A5E FUN_00017a5e
//   0x00017C52 render_menu_text_followup
//   0x00017C84 set_hud_selection_value
//   0x00017E04 refresh_hud_grid_if_ready
//   0x00019DAC FUN_00019dac
//   0x0001B706 prepare_menu_offset_once


void FUN_00019d66(void)

{
  if (*(char *)((int)&DAT_00fffb30 + (int)DAT_00fff102) < '\x01') {
    *(undefined1 *)((int)&DAT_00fffb30 + (int)DAT_00fff102) = 0;
    set_hud_selection_value();
    render_menu_text_followup();
    refresh_hud_grid_if_ready();
    prepare_menu_offset_once();
    return;
  }
  set_hud_selection_value();
  render_menu_text_followup();
  FUN_00019dac();
  render_menu_text_progression();
  FUN_00017a5e();
  return;
}


