// Ghidra headless export
// Entry: 0x00017708
// Name: render_field_hud
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block
//   0x00001728 render_hud_sprite_stream
//   0x00017D72 draw_hud_panel
//   0x00017D90 advance_hud_panel
//   0x00017F8E draw_hud_footer
//   0x00019856 refresh_field_hud
//   0x00019BAE select_hud_mode


void render_field_hud(void)

{
  short sVar1;
  
  VDP_CTRL = 0x8134;
  render_hud_sprite_stream();
  sVar1 = 4;
  do {
    upload_hud_tile_block();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  refresh_field_hud();
  if ((DAT_00ffdf84 != '\0') && (sVar1 = select_hud_mode(), sVar1 == 8)) {
    draw_hud_panel();
    advance_hud_panel();
    draw_hud_footer();
  }
  return;
}


