// Ghidra headless export
// Entry: 0x00001A9E
// Name: run_title_loop
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000E34 FUN_00000e34
//   0x0000131C step_palette_transition
//   0x0000195C sample_controller_input
//   0x00001BAC finalize_title_input_sequence
//   0x00001BEA record_title_input_edge
//   0x00001C1E advance_title_text_state
//   0x00001C6A advance_title_sprite_state
//   0x00001D7A setup_title_state
//   0x00001DCA draw_title_prompt_variant_a
//   0x00001DE0 draw_title_prompt_variant_b
//   0x00001E4E reveal_title_logo_column
//   0x00002634 cycle_title_circuit_lights
//   0x000026D0 advance_title_animation_timer


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void run_title_loop(void)

{
  ushort uVar1;
  
  DAT_00fff101 = 0;
  setup_title_state();
  DAT_00ffe00a = 0;
  DAT_00ffdd0e = 0;
  sample_controller_input();
  _DAT_00ffddc2 = 0;
  DAT_00ffdd2c = 0x5a;
  DAT_00ffdd2e = 0;
  _DAT_00ffdd3c = 0;
  DAT_00ffdd5a = 0;
  DAT_00ffdd5c = 0x5a;
  DAT_00ffdd42 = 0x98;
  DAT_00ffdd40 = 0;
  DAT_00ffdd46 = 0;
  uVar1 = 0xe10;
  do {
    wait_for_vblank();
    advance_title_text_state();
    cycle_title_circuit_lights();
    advance_title_animation_timer();
    sample_controller_input();
    record_title_input_edge();
    latch_controller_events();
    DAT_00ffdd2c = DAT_00ffdd2c + -1;
    if ((DAT_00ffdd2c < 0) && (DAT_00fff1e6 < 0x26)) {
      DAT_00ffdd40 = DAT_00ffdd40 + -1;
      if (DAT_00ffdd40 < 0) {
        DAT_00ffdd40 = 1;
        reveal_title_logo_column();
      }
      advance_title_sprite_state();
      DAT_00ffef14 = DAT_00ffef14 | 2;
    }
    else {
      FUN_00000e34();
    }
    if (uVar1 < 0xc6c) {
      if ((uVar1 & 0x3f) < 10) {
        draw_title_prompt_variant_b();
      }
      else {
        draw_title_prompt_variant_a();
      }
      if (_DAT_00ffddc2 != 0) break;
    }
    uVar1 = uVar1 - 1;
  } while (uVar1 != 0xffff);
  sample_controller_input();
  step_palette_transition();
  finalize_title_input_sequence();
  return;
}


