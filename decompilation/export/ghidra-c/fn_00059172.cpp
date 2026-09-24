// Ghidra headless export
// Entry: 0x00059172
// Name: FUN_00059172
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000D94 wait_for_controller_event
//   0x0000195C sample_controller_input
//   0x0000C0DA render_title_text_source
//   0x0000C376 move_wrapped_menu_cursor
//   0x0000C3E6 FUN_0000c3e6
//   0x00059360 coordinate_save_load


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00059172(void)

{
  undefined1 uVar1;
  
  render_title_text_source();
  DAT_00fff0dc = 0;
  do {
    uVar1 = 0;
    move_wrapped_menu_cursor();
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
      if (((DAT_00ffdd22._0_1_ & 0x40) != 0) ||
         (((DAT_00ffdd22._0_1_ & 0x10) != 0 || ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) {
        FUN_0000c3e6();
        coordinate_save_load();
        if ((bool)uVar1) {
          DAT_00fff0dc = 0xff;
        }
        else {
          _DAT_00ffe108 = 0xffff;
        }
        render_title_text_source();
        wait_for_controller_event();
        return;
      }
    }
  } while( true );
}


