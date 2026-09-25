// Ghidra headless export
// Entry: 0x00057D7E
// Name: convert_intro_routine_0x00057d7e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x0000C216 build_grouped_decimal_glyphs
//   0x00011190 transfer_menu_glyph_triple
//   0x00057A6E convert_intro_routine_0x00057a6e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_intro_routine_0x00057d7e(void)

{
  short sVar1;
  char unaff_D7b;
  int extraout_A0;
  
  if ((char)(*(char *)(DAT_00fffc04 + 0x9b) + -2) < '\0') {
    render_encoded_title_text();
    render_encoded_title_text();
    DAT_00fff0c2 = DAT_00fff10e + 1;
    sVar1 = *DAT_00fff10e;
    do {
      DAT_00fff0ae = unaff_D7b;
      if (unaff_D7b == *(char *)DAT_00fff0c2) {
        if ((char)((byte)*DAT_00fff0c2 - *(char *)(DAT_00fffc04 + 0x7f)) < '\0') {
          _DAT_00fff0be =
               (uint)(ushort)((byte)*DAT_00fff0c2 + 1) * (uint)*(ushort *)(extraout_A0 + 2);
          goto LAB_00057e38;
        }
        render_encoded_title_text();
        goto LAB_00057e9a;
      }
      DAT_00fff0c2 = DAT_00fff0c2 + 1;
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
    _DAT_00fff0be = (uint)*(ushort *)(extraout_A0 + 2);
LAB_00057e38:
    render_encoded_title_text();
    render_encoded_title_text();
    build_grouped_decimal_glyphs();
    convert_intro_routine_0x00057a6e(0xc428);
    build_grouped_decimal_glyphs();
LAB_00057e9a:
    transfer_menu_glyph_triple();
  }
  else {
    render_encoded_title_text();
  }
  return;
}


