// Ghidra headless export
// Entry: 0x000039F2
// Name: refresh_field_animation_assets
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000033AA draw_field_asset_grid
//   0x00003548 draw_hud_panel_triplet
//   0x000038FA run_entity_animation_triplet


void refresh_field_animation_assets(void)

{
  draw_field_asset_grid();
  draw_hud_panel_triplet();
  run_entity_animation_triplet();
  return;
}


