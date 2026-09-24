// Ghidra headless export
// Entry: 0x000586CA
// Name: FUN_000586ca
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_title_text_source
//   0x0000C216 build_grouped_decimal_glyphs
//   0x000130C0 FUN_000130c0
//   0x00057A44 FUN_00057a44
//   0x00057A6E FUN_00057a6e
//   0x0005878E FUN_0005878e


void FUN_000586ca(void)

{
  uint in_D0;
  int in_A0;
  char *pcVar1;
  
  pcVar1 = (char *)((uint)DAT_00fff0d0 * 3 + (in_D0 & 0xffff) * 0x18 + in_A0);
  if (-1 < *pcVar1) {
    if (*pcVar1 == 'E') {
      write_vdp_grid();
      render_title_text_source();
      FUN_00057a6e();
      build_grouped_decimal_glyphs();
      render_title_text_source();
      return;
    }
    FUN_000130c0();
    FUN_0005878e();
    render_title_text_source();
    FUN_00057a44();
    build_grouped_decimal_glyphs();
  }
  render_title_text_source();
  return;
}


