// Ghidra headless export
// Entry: 0x00011136
// Name: render_menu_tile_rows
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003602 run_hud_panel_post_hook
//   0x00011190 transfer_menu_glyph_triple


ushort render_menu_tile_rows(void)

{
  ushort uVar1;
  ushort uVar2;
  
  uVar2 = 7;
  do {
    run_hud_panel_post_hook();
    transfer_menu_glyph_triple();
    uVar1 = uVar2 >> 1;
    uVar2 = uVar2 - 1;
  } while (uVar2 != 0xffff);
  return uVar1;
}


