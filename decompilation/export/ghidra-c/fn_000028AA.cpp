// Ghidra headless export
// Entry: 0x000028AA
// Name: FUN_000028aa
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000334C upload_field_asset_sequence
//   0x00003404 upload_field_asset_setup
//   0x000039F2 refresh_field_animation_assets
//   0x0000C0C2 load_title_font


void FUN_000028aa(void)

{
  upload_field_asset_setup();
  upload_field_asset_sequence();
  refresh_field_animation_assets();
  load_title_font();
  return;
}


