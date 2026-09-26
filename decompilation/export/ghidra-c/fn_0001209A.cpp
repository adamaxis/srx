// Ghidra headless export
// Entry: 0x0001209A
// Name: advance_menu_text_cursor
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA render_encoded_title_text
//   0x000110E8 enter_active_runner_hud_panel


void advance_menu_text_cursor(void)

{
  short *psVar1;
  
  do {
    psVar1 = &DAT_00ffdd48;
    if (DAT_00ffdd28 == 0) {
      DAT_00ffdd28 = -1;
    }
    else {
      DAT_00ffdd28 = 0;
      psVar1 = (short *)&DAT_00ffdd4c;
    }
  } while (*psVar1 < 0);
  render_encoded_title_text();
  enter_active_runner_hud_panel();
  return;
}

