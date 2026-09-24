// Ghidra headless export
// Entry: 0x00018AA6
// Name: FUN_00018aa6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x0000C0DA render_title_text_source
//   0x0000C216 FUN_0000c216
//   0x0000C376 move_wrapped_menu_cursor
//   0x0001882E FUN_0001882e
//   0x0001897C FUN_0001897c
//   0x000196F2 FUN_000196f2


void FUN_00018aa6(void)

{
  short sVar1;
  
  DAT_00fff0dc = '\0';
  FUN_0001897c();
  render_title_text_source();
  do {
    move_wrapped_menu_cursor();
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      FUN_000196f2();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
      if (((DAT_00ffdd22._0_1_ & 0x40) != 0) ||
         (((DAT_00ffdd22._0_1_ & 0x10) != 0 || ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) {
        if (DAT_00fff0dc == '\0') {
          FUN_0001897c();
          render_title_text_source();
          render_title_text_source();
          FUN_0000c216();
          FUN_0000c216();
          FUN_0000c216();
          FUN_0000c216();
          FUN_0000c216();
          FUN_0000c216();
          sVar1 = 3;
          do {
            FUN_0000c216();
            sVar1 = sVar1 + -1;
          } while (sVar1 != -1);
          return;
        }
        FUN_0001897c();
        render_title_text_source();
        if (DAT_00fff0e3 < '\0') {
          render_title_text_source();
          if (DAT_00fffbf2 < '\0') {
            render_title_text_source();
          }
          else {
            render_title_text_source();
          }
          render_title_text_source();
          render_title_text_source();
          return;
        }
        if (DAT_00fff0e3 < '\x01') {
          render_title_text_source();
          return;
        }
        render_title_text_source();
        return;
      }
      if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
        FUN_0001882e();
        return;
      }
    }
  } while( true );
}


