// Ghidra headless export
// Entry: 0x00017998
// Name: FUN_00017998
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00017C3C FUN_00017c3c


void FUN_00017998(void)

{
  short unaff_D3w;
  undefined1 uVar1;
  
  uVar1 = 0;
  render_title_text_source();
  FUN_00017c3c();
  if (!(bool)uVar1) {
    render_title_text_source();
    if (-1 < (short)(unaff_D3w + -3)) {
      render_title_text_source();
    }
    if (-1 < (short)(unaff_D3w + -2)) {
      build_grouped_decimal_glyphs();
      if (-1 < (short)(unaff_D3w + -3)) {
        return;
      }
      goto LAB_00017a46;
    }
  }
  render_title_text_source();
LAB_00017a46:
  render_title_text_source();
  return;
}


