// Ghidra headless export
// Entry: 0x0001F246
// Name: run_mode_exit_services
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0C2 load_title_font
//   0x00010952 reset_mode_display_buffers
//   0x00010984 transfer_menu_asset_batch_nine_optional
//   0x0001F292 prepare_mode_exit_resources
//   0x0001F3C4 run_mode_text_prompt
//   0x0005670E refresh_mode_palette_data


void run_mode_exit_services(void)

{
  code *in_A1;
  
  VDP_CTRL = 0x9100;
  load_title_font();
  transfer_menu_asset_batch_nine_optional();
  prepare_mode_exit_resources();
  if (DAT_00fff10c != '\0') {
    (*in_A1)();
  }
  DAT_00fff10c = 0;
  run_mode_text_prompt();
  reset_mode_display_buffers();
  refresh_mode_palette_data();
  VDP_CTRL = 0x9191;
  return;
}


