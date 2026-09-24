// Ghidra headless export
// Entry: 0x00057D7E
// Name: FUN_00057d7e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x0000C216 FUN_0000c216
//   0x00011190 FUN_00011190
//   0x00057A6E FUN_00057a6e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00057d7e(void)

{
  short sVar1;
  char unaff_D7b;
  int extraout_A0;
  
  if ((char)(*(char *)(DAT_00fffc04 + 0x9b) + -2) < '\0') {
    render_title_text_source();
    render_title_text_source();
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
        render_title_text_source();
        goto LAB_00057e9a;
      }
      DAT_00fff0c2 = DAT_00fff0c2 + 1;
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
    _DAT_00fff0be = (uint)*(ushort *)(extraout_A0 + 2);
LAB_00057e38:
    render_title_text_source();
    render_title_text_source();
    FUN_0000c216();
    FUN_00057a6e(0xc428);
    FUN_0000c216();
LAB_00057e9a:
    FUN_00011190();
  }
  else {
    render_title_text_source();
  }
  return;
}


