// Ghidra headless export
// Entry: 0x00054A8E
// Name: FUN_00054a8e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000C24 wait_for_vblank_frames
//   0x00000D3A sample_source_random_bound
//   0x00000D94 wait_for_controller_event
//   0x0000C0DA render_encoded_title_text
//   0x0001989A FUN_0001989a
//   0x000549E4 write_menu_repeated_grid_service
//   0x000549F6 select_display_status


void FUN_00054a8e(void)

{
  char cVar1;
  
  cVar1 = sample_source_random_bound();
  if (DAT_00fff0a8 <= cVar1) {
    return;
  }
  DAT_00fff0a8 = 0;
  FUN_0001989a();
  if (DAT_00fff0b1 == '\0') {
    select_display_status();
    render_encoded_title_text();
    wait_for_vblank_frames();
    wait_for_controller_event();
  }
  write_menu_repeated_grid_service();
  return;
}


