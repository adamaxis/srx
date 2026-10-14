// Ghidra headless export
// Entry: 0x00019238
// Name: run_matrix_program_slot_screen
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000C0DA render_encoded_title_text
//   0x0000C376 move_wrapped_menu_cursor
//   0x0001882E render_matrix_panel
//   0x00018998 write_hud_repeated_grid_variant_b
//   0x00018A92 matrix_program_index
//   0x000194AC advance_hud_selection_row
//   0x00019504 write_hud_panel_grid
//   0x0001952C matrix_program_storage_usage
//   0x0001957E calculate_loaded_program_capacity
//   0x000195D8 refresh_matrix_program_slot
//   0x0001962A apply_menu_slot_command
//   0x000196F2 rotate_hud_history
//   0x00019736 clear_ready_menu_slot_timer


void run_matrix_program_slot_screen(void)

{
  short extraout_D1w;
  short unaff_D6w;
  short unaff_D7w;
  short sVar1;
  undefined1 *puVar2;
  
  matrix_program_index();
  if ((&DAT_00fffbcc)[unaff_D7w] != '\0') {
    DAT_00ffdd50 = 0;
    DAT_00fff0dc = '\0';
    write_hud_repeated_grid_variant_b();
    matrix_program_index();
    puVar2 = &DAT_00fffbbe;
    sVar1 = 4;
    do {
      if ((char)unaff_D7w == puVar2[0x1a]) {
        DAT_00fff0d4 = '\x04' - (char)sVar1;
        render_encoded_title_text();
        render_encoded_title_text();
        render_encoded_title_text();
        DAT_00ffdd50 = -1;
        DAT_00fff0dc = '\x01';
        advance_hud_selection_row();
        render_encoded_title_text();
        do {
          move_wrapped_menu_cursor();
          while( true ) {
            wait_for_vblank();
            latch_controller_events();
            sample_controller_input();
            rotate_hud_history();
            if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
            if (((DAT_00ffdd22._0_1_ & 0x40) != 0) ||
               (((DAT_00ffdd22._0_1_ & 0x10) != 0 || ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) {
              if (DAT_00fff0dc != '\0') goto LAB_00019428;
              if (DAT_00ffdd50 == 0) goto LAB_00019410;
              apply_menu_slot_command();
              goto LAB_00019428;
            }
            if ((DAT_00ffdd22._0_1_ & 0x80) != 0) goto LAB_00019428;
          }
        } while( true );
      }
      puVar2 = puVar2 + 1;
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
    puVar2 = &DAT_00fffbbe;
    sVar1 = 4;
    while (-1 < (char)puVar2[0x1a]) {
      puVar2 = puVar2 + 1;
      sVar1 = sVar1 + -1;
      if (sVar1 == -1) {
        render_encoded_title_text();
        render_encoded_title_text();
        return;
      }
    }
    DAT_00fff0d4 = '\x04' - (char)sVar1;
    calculate_loaded_program_capacity();
    matrix_program_index();
    matrix_program_storage_usage();
    if ((short)(unaff_D6w - extraout_D1w) < 0) {
      render_encoded_title_text();
      render_encoded_title_text();
      return;
    }
LAB_00019410:
    clear_ready_menu_slot_timer();
    refresh_matrix_program_slot();
LAB_00019428:
    write_hud_panel_grid();
    render_matrix_panel();
  }
  return;
}

