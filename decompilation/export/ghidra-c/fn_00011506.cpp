// Ghidra headless export
// Entry: 0x00011506
// Name: FUN_00011506
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00011442 reset_and_upload_menu_tile_block
//   0x00011454 reset_menu_upload_cursor_and_upload
//   0x00011592 FUN_00011592
//   0x000119CE FUN_000119ce


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00011506(void)

{
  char cVar1;
  undefined1 uVar2;
  
  if (-1 < (char)(DAT_00fff0d0 + -2)) {
    if ((((DAT_00ffdd22 & 2) == 0) && ((DAT_00ffdd22 & 8) == 0)) && ((DAT_00ffdd22 & 4) == 0)) {
      if (-1 < DAT_00fff115) {
        reset_and_upload_menu_tile_block();
        DAT_00fff115 = 0xff;
        FUN_000119ce();
        DAT_00fff115 = 0;
        reset_menu_upload_cursor_and_upload();
        return;
      }
      DAT_00fff115 = '\0';
    }
    return;
  }
  cVar1 = reset_and_upload_menu_tile_block();
  DAT_00fff0d1 = cVar1 + DAT_00fff0d1;
  if (DAT_00fff0d1 < '\0') {
    DAT_00fff0d1 = 0;
    FUN_00011592();
    return;
  }
  if (DAT_00fff0d1 == '\x04') {
    DAT_00fff0d1 = '\x03';
  }
  DAT_00ffdd02 = 1;
  build_title_sprite_records();
  _DAT_00ffd980 = 1;
  uVar2 = (DAT_00ffef14 & 2) == 0;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  prepare_menu_grid_variant_b();
  read_entity_action_code();
  if (!(bool)uVar2) {
    cVar1 = FUN_00012a8c();
    if ((char)(cVar1 + -0x10) < '\0') {
      service_entity_action_menu();
    }
    prepare_menu_dialog();
    return;
  }
  render_title_text_source();
  return;
}


