// Ghidra headless export
// Entry: 0x000186FA
// Name: refresh_field_hud_assets
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001530 decode_and_upload_source_stream
//   0x00001728 render_hud_sprite_stream
//   0x00017708 render_field_hud
//   0x000194AC advance_hud_selection_row
//   0x00026958 FUN_00026958


void refresh_field_hud_assets(void)

{
  render_field_hud();
  DAT_00fff0d4 = 0;
  advance_hud_selection_row();
  decode_and_upload_source_stream();
  FUN_00026958();
  render_hud_sprite_stream();
  render_hud_sprite_stream();
  return;
}


