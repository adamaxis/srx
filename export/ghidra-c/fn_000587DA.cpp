// Ghidra headless export
// Entry: 0x000587DA
// Name: render_field_mode_selection_presentation
// Decompile status: source-shaped annotation
//
// The native conversion is
// ghidra_conversion::render_field_mode_selection_presentation(). It preserves
// the signed mode-result branches, saved cursor, grid/glyph batch counts, and
// the typed selection-record boundary without retaining VDP or ROM pointers.
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00011190 transfer_menu_glyph_triple
//   0x0005763C select_menu_resource
//   0x00058A2E render_alternating_field_mode_records
//   0x00058AA4 render_field_mode_glyph_row
//   0x00058B1E render_field_mode_grid_pair_batches


void render_field_mode_selection_presentation(void)

{
  short sVar1;
  short sVar2;
  
  DAT_00fff0b8 = DAT_00fff0d0;
  if (DAT_00fff0b7 < '\0') {
    if (((DAT_00fff0d0 != '\0') && (-1 < (char)(DAT_00fff0d0 + -2))) && (DAT_00fff0d0 != '\x02')) {
      if (DAT_00fff0d0 == '\x03') {
        render_field_mode_grid_pair_batches();
        return;
      }
LAB_0005884c:
      render_field_mode_glyph_row();
      return;
    }
  }
  else {
    if (DAT_00fff0b7 != '\0') {
      if (DAT_00fff0b7 == '\x01') {
        sVar1 = 7;
        do {
          write_vdp_grid();
          sVar1 = sVar1 + -1;
        } while (sVar1 != -1);
        return;
      }
      if (-1 < (char)(DAT_00fff0d0 + -3)) {
        if (DAT_00fff0d0 == '\x03') {
          sVar1 = 7;
          do {
            write_vdp_grid();
            sVar1 = sVar1 + -1;
          } while (sVar1 != -1);
          return;
        }
        if (DAT_00fff0d0 == '\x05') {
          sVar1 = 4;
          sVar2 = 2;
        }
        else {
          sVar1 = 3;
          sVar2 = 3;
        }
        do {
          write_vdp_grid();
          sVar1 = sVar1 + -1;
        } while (sVar1 != -1);
        do {
          write_vdp_grid();
          sVar2 = sVar2 + -1;
        } while (sVar2 != -1);
        return;
      }
      select_menu_resource();
      sVar1 = 7;
      do {
        write_vdp_grid();
        sVar1 = sVar1 + -1;
      } while (sVar1 != -1);
      return;
    }
    if (DAT_00fff0d0 != '\x01') {
      if ((char)(DAT_00fff0d0 + -1) < '\0') {
        sVar1 = 7;
        do {
          transfer_menu_glyph_triple();
          sVar1 = sVar1 + -1;
        } while (sVar1 != -1);
        return;
      }
      if (DAT_00fff0d0 == '\x03') goto LAB_0005884c;
    }
  }
  render_alternating_field_mode_records();
  return;
}

