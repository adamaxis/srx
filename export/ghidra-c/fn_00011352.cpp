// Ghidra headless export
// Entry: 0x00011352
// Name: apply_armor_selection
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x00055E92 subtract_armor_primary_modifier
//   0x00055EB4 subtract_armor_secondary_modifier


void apply_armor_selection(void)

{
  char in_D0b;
  short unaff_D7w;
  int in_A0;
  int extraout_A0;
  
  if (DAT_00fff0b1 < '\0') {
    DAT_00ffef96 = *(undefined1 *)(in_A0 + 0x76);
    *(char *)(in_A0 + 0x76) = in_D0b + -0x2b;
    DAT_00ffef97 = *(undefined1 *)(in_A0 + 0x6e + (int)unaff_D7w);
    *(char *)(in_A0 + 0x6e + (int)unaff_D7w) = in_D0b + -0x2b;
  }
  else if (*(char *)(in_A0 + 0x76) == '\0') {
    render_encoded_title_text();
    return;
  }
  render_encoded_title_text();
  render_encoded_title_text();
  subtract_armor_secondary_modifier();
  write_vdp_grid();
  subtract_armor_primary_modifier();
  write_vdp_grid();
  if (DAT_00fff0b1 < '\0') {
    DAT_00fff0b1 = '\0';
    *(undefined1 *)(extraout_A0 + 0x76) = DAT_00ffef96;
    *(undefined1 *)(extraout_A0 + 0x6e + (int)unaff_D7w) = DAT_00ffef97;
  }
  return;
}


