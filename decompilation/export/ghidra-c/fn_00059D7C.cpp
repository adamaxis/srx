// Ghidra headless export
// Entry: 0x00059D7C
// Name: FUN_00059d7c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x000012F6 advance_title_palette_fade
//   0x00001406 prepare_title_resources
//   0x0000195C sample_controller_input
//   0x0000C0C2 load_title_font
//   0x00059F7A reset_title_intro_overlay_schedule
//   0x00059F8E advance_title_intro_overlay
//   0x0005A0A4 reset_title_intro_sequence
//   0x0005A0B2 advance_title_intro_sequence
//   0x0005A1EA FUN_0005a1ea
//   0x0005A244 reset_title_intro_persistent_sequence
//   0x0005A37C FUN_0005a37c
//   0x0005A3B0 reset_title_intro_motion
//   0x0005A4B4 reset_title_intro_dual_sprites
//   0x0005A572 reset_title_intro_timed_sprites
//   0x0005A61A reset_title_intro_targeted_sweep
//   0x0005A70E reset_title_intro_linear_sweep_a
//   0x0005A7EA reset_title_intro_linear_sweep_b
//   0x0005A8C8 reset_title_intro_final_projection
//   0x0005ADFE render_title_intro_late_text_batch
//   0x0005AE3A render_title_intro_scroll_text_batch
//   0x0005AF2A advance_title_intro_schedule


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00059d7c(void)

{
  short sVar1;
  
  load_title_font();
  DAT_00ffdd32 = &DAT_0005adf0;
  DAT_00ffdd36 = DAT_0005adf0;
  prepare_title_resources();
  DAT_00ffdd08 = -0x40;
  DAT_00ffdd0c = 0;
  DAT_00ffdd0a = -0x20;
  DAT_00ffdd0e = 0;
  DAT_00ffef14 = DAT_00ffef14 | 0xc;
  DAT_00ffdd46 = 0x138;
  DAT_00ffdd48 = 0xf1;
  DAT_00ffdd02 = 1;
  FUN_0005a37c();
  _DAT_00ffd980 = 0;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  advance_title_palette_fade();
  sVar1 = 0xa0;
  do {
    wait_for_vblank();
    sample_controller_input();
    latch_controller_events();
    advance_title_intro_schedule();
    if (DAT_00ffdd08 != 0) {
      if (DAT_00ffdd46 != 0x178) {
        DAT_00ffdd46 = DAT_00ffdd46 + 1;
      }
      DAT_00ffdd08 = DAT_00ffdd08 + 1;
      DAT_00ffef14 = DAT_00ffef14 | 4;
    }
    if ((DAT_00ffdd0a != 0) && ((DAT_00ffdf88 & 1) == 0)) {
      if (DAT_00ffdd48 != 0xd1) {
        DAT_00ffdd48 = DAT_00ffdd48 + -1;
      }
      DAT_00ffdd0a = DAT_00ffdd0a + 1;
      DAT_00ffef14 = DAT_00ffef14 | 8;
    }
    DAT_00ffdd02 = 1;
    FUN_0005a37c();
    _DAT_00ffd980 = 0;
    DAT_00ffef14 = DAT_00ffef14 | 2;
  } while ((_DAT_00ffddc2 == 0) && (sVar1 = sVar1 + -1, sVar1 != -1));
  DAT_00fff11e = DAT_00ffdf88;
  reset_title_intro_overlay_schedule();
  reset_title_intro_sequence();
  reset_title_intro_persistent_sequence();
  reset_title_intro_motion();
  reset_title_intro_dual_sprites();
  reset_title_intro_timed_sprites();
  reset_title_intro_targeted_sweep();
  reset_title_intro_linear_sweep_a();
  reset_title_intro_linear_sweep_b();
  reset_title_intro_final_projection();
  FUN_0005a1ea();
  sample_controller_input();
  sVar1 = 0x1f0;
  do {
    wait_for_vblank();
    advance_title_intro_overlay();
    advance_title_intro_sequence();
    sample_controller_input();
    latch_controller_events();
    advance_title_intro_schedule();
    FUN_0005a1ea();
    if (sVar1 == 0x164) {
      render_title_intro_scroll_text_batch();
      render_title_intro_late_text_batch();
    }
  } while ((_DAT_00ffddc2 == 0) && (sVar1 = sVar1 + -1, sVar1 != -1));
  return;
}


