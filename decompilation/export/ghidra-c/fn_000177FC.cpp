// Ghidra headless export
// Entry: 0x000177FC
// Name: run_menu_hud_tick_loop
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00017CC8 advance_hud_sprite_audio_pulse
//   0x00017FDA advance_menu_random_glitch
//   0x000194AC advance_hud_selection_row
//   0x0001C3FA convert_world_routine_0x0001c3fa
//   0x000266E4 convert_world_routine_0x000266e4
//   0x0002679E convert_world_routine_0x0002679e
//   0x000268E6 append_world_sprite_records


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void run_menu_hud_tick_loop(void)

{
  short sVar1;
  
  sVar1 = 0x3c;
  do {
    wait_for_vblank();
    latch_controller_events();
    advance_hud_sprite_audio_pulse();
    advance_menu_random_glitch();
    DAT_00ffdd02 = 1;
    convert_world_routine_0x0001c3fa();
    if (DAT_00fff0e1 != '\0') {
      append_world_sprite_records();
    }
    _DAT_00ffd980 = 1;
    DAT_00ffef14 = DAT_00ffef14 | 2;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  convert_world_routine_0x000266e4();
  convert_world_routine_0x0002679e();
  DAT_00fff0d4 = 0;
  advance_hud_selection_row();
  return;
}


