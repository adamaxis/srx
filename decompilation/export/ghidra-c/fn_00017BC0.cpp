// Ghidra headless export
// Entry: 0x00017BC0
// Name: FUN_00017bc0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00012C5A FUN_00012c5a
//   0x00017C52 FUN_00017c52


void FUN_00017bc0(void)

{
  if (-1 < (char)(&DAT_00fffbd8)[(short)(ushort)DAT_00fff0d4]) {
    render_title_text_source();
    build_grouped_decimal_glyphs();
    FUN_00017c52();
    return;
  }
  FUN_00012c5a();
  return;
}


