// Ghidra headless export
// Entry: 0x0001B282
// Name: advance_matrix_program_transition
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x0001792E render_menu_hud_rows
//   0x00017BC0 refresh_menu_number_or_grid
//   0x00019632 clear_matrix_program_slot
//   0x00019AEE refresh_mode_glyph
//   0x0001A908 initialize_world_effect_mode


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_matrix_program_transition(void)

{
  DAT_00ffe0da = DAT_00ffe0da + -1;
  if (DAT_00ffe0da < 0) {
    DAT_00ffe0da = 0xc;
    DAT_00ffe0dc = DAT_00ffe0dc + 1;
    if (DAT_00ffe0dc == 6) {
      clear_matrix_program_slot();
      refresh_menu_number_or_grid();
    }
    if (0xb < DAT_00ffe0dc) {
      refresh_mode_glyph();
      _DAT_00ffe0de = 0;
      render_menu_hud_rows();
      initialize_world_effect_mode();
      return;
    }
  }
  build_title_sprite_records();
  return;
}

