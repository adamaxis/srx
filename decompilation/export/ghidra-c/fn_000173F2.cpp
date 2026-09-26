// Ghidra headless export
// Entry: 0x000173F2
// Name: prepare_menu_mode_resources
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001406 prepare_title_resources
//   0x0001745E run_field_mode_hud_scene


void prepare_menu_mode_resources(void)

{
  if (DAT_00ffe16f == '\0') {
    DAT_00fff0a0 = -1;
  }
  else {
    DAT_00fff0a0 = '\x01';
  }
  DAT_00fff0b0 = 0xff;
  run_field_mode_hud_scene();
  DAT_00fff0e3 = 0;
  DAT_00fff0b0 = 0;
  VDP_CTRL = 0x8134;
  prepare_title_resources();
  if ((DAT_00ffdf40 == 2) && (DAT_00fff0a0 == '\x01')) {
    DAT_00ffe191 = 1;
  }
  return;
}


