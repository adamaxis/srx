// Ghidra headless export
// Entry: 0x0000FBA6
// Name: run_magic_menu
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000D94 wait_for_controller_event
//   0x00000FFE clear_and_fill_plane_a
//   0x00001358 copy_source_word_block
//   0x00001530 decode_and_upload_source_stream
//   0x00001728 render_hud_sprite_stream
//   0x0000195C sample_controller_input
//   0x0000C0DA render_encoded_title_text
//   0x0000FDFE advance_menu_sprite_overlay
//   0x0000FEBE move_selection_column
//   0x0000FF06 move_selection_row
//   0x0000FF5A run_equipment_selection
//   0x00010276 run_menu_text_display_sequence
//   0x000106DA prepare_grid_then_write
//   0x00011190 transfer_menu_glyph_triple
//   0x00011900 prepare_menu_row_grid
//   0x00011928 render_menu_row
//   0x00012C36 prepare_menu_grid_variant_b
//
// Source behavior: the spell-panel branch performs one-time setup, then the
// frame loop gives column navigation priority over row navigation, action,
// and bit-7 exit. Navigation redraws and clears the cursor timer; action
// redraws without that timer reset. The fallback branch is separate.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void run_magic_menu(void)

{
  undefined2 uVar1;
  short sVar2;
  
  uVar1 = _DAT_00fff0d0;
  DAT_00fff0db = 0;
  if (((char)(*(char *)(DAT_00fffc04 + 0x9b) + -2) < '\0') &&
     (*(char *)(DAT_00fffc04 + 0x7f) != '\0')) {
    VDP_CTRL = 0x8134;
    _DAT_00fff0d0 = 0;
    copy_source_word_block();
    DAT_00ffef14 = DAT_00ffef14 | 1;
    clear_and_fill_plane_a();
    decode_and_upload_source_stream();
    render_hud_sprite_stream();
    DAT_00fff10e = &DAT_00fff9fa + (uint)*(ushort *)(DAT_00fffc04 + 0xa0) * 0x11;
    sVar2 = *DAT_00fff10e;
    do {
      transfer_menu_glyph_triple();
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
    run_menu_text_display_sequence();
    VDP_CTRL = 0x8174;
    do {
      while( true ) {
        while( true ) {
          while( true ) {
            advance_menu_sprite_overlay();
            wait_for_vblank();
            latch_controller_events();
            sample_controller_input();
            if (((DAT_00ffdd22._0_1_ & 4) == 0) && ((DAT_00ffdd22._0_1_ & 8) == 0)) break;
            move_selection_column();
            run_menu_text_display_sequence();
            DAT_00fff0da = 0;
          }
          if (((DAT_00ffdd22._0_1_ & 1) == 0) && ((DAT_00ffdd22._0_1_ & 2) == 0)) break;
          move_selection_row();
          run_menu_text_display_sequence();
          DAT_00fff0da = 0;
        }
        if ((((DAT_00ffdd22._0_1_ & 0x20) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) &&
           ((DAT_00ffdd22._0_1_ & 0x40) == 0)) break;
        run_equipment_selection();
        run_menu_text_display_sequence();
      }
    } while ((DAT_00ffdd22._0_1_ & 0x80) == 0);
    _DAT_00fff0d0 = uVar1;
    return;
  }
  prepare_menu_row_grid();
  prepare_menu_grid_variant_b();
  render_encoded_title_text();
  prepare_grid_then_write();
  prepare_grid_then_write();
  wait_for_controller_event();
  render_menu_row();
  return;
}


