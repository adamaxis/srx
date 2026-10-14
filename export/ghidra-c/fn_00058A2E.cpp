// Ghidra headless export
// Entry: 0x00058A2E
// Name: render_alternating_field_mode_records
// Decompile status: source-shaped annotation
//
// The native conversion is
// ghidra_conversion::render_alternating_field_mode_records(). The source
// three-byte A0 records are represented by eight typed leading markers.
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00011190 transfer_menu_glyph_triple


void render_alternating_field_mode_records(void)

{
  short sVar1;
  char *in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int iVar2;
  
  sVar1 = 7;
  do {
    if ((*in_A0 < '\0') || (*in_A0 != 'E')) {
      transfer_menu_glyph_triple();
      iVar2 = extraout_A0_00;
    }
    else {
      write_vdp_grid();
      iVar2 = extraout_A0;
    }
    in_A0 = (char *)(iVar2 + 3);
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}

