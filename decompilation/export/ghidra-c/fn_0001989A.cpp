// Ghidra headless export
// Entry: 0x0001989A
// Name: FUN_0001989a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000C24 FUN_00000c24
//   0x00000D94 FUN_00000d94
//   0x0000C0DA render_title_text_source
//   0x00018776 FUN_00018776
//   0x00026A00 FUN_00026a00
//   0x000530DE FUN_000530de


void FUN_0001989a(void)

{
  char in_D1b;
  
  DAT_00fff0a0 = in_D1b + DAT_00fff0a0;
  if ((DAT_00fff0a0 != '\0') && (DAT_00fff0a0 != '\x01')) {
    if (DAT_00fff0a0 < '\x02') {
      DAT_00fff0a0 = -1;
    }
    else {
      DAT_00fff0a0 = '\x01';
      if (DAT_00fff0a6 != '\0') {
        FUN_00018776();
        FUN_00026a00();
        render_title_text_source();
        FUN_00000c24();
        FUN_00000d94();
        return;
      }
    }
  }
  if ((-1 < DAT_00fff0a6) && (render_title_text_source(), DAT_00ffe189 != DAT_00fff0a0)) {
    DAT_00ffe189 = DAT_00fff0a0;
    FUN_000530de();
  }
  return;
}


