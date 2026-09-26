// Ghidra headless export
// Entry: 0x00017998
// Name: render_menu_text_progression
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00017C3C read_menu_entry_profile_eligibility


void render_menu_text_progression(void)

{
  short unaff_D3w;
  undefined1 uVar1;
  
  uVar1 = 0;
  render_encoded_title_text();
  read_menu_entry_profile_eligibility();
  if (!(bool)uVar1) {
    render_encoded_title_text();
    if (-1 < (short)(unaff_D3w + -3)) {
      render_encoded_title_text();
    }
    if (-1 < (short)(unaff_D3w + -2)) {
      build_grouped_decimal_glyphs();
      if (-1 < (short)(unaff_D3w + -3)) {
        return;
      }
      goto LAB_00017a46;
    }
  }
  render_encoded_title_text();
LAB_00017a46:
  render_encoded_title_text();
  return;
}


