// Ghidra headless export
// Entry: 0x00012062
// Name: refresh_menu_text
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_title_text_source
//   0x00012C36 FUN_00012c36


void refresh_menu_text(void)

{
  int extraout_A0;
  
  if (*(char *)(DAT_00fffc04 + 0x4f) < '\x01') {
    FUN_00012c36();
    render_title_text_source();
    render_title_text_source();
    *(undefined1 *)(extraout_A0 + 0x4f) = 0;
  }
  return;
}


