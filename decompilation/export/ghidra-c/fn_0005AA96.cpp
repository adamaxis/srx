// Ghidra headless export
// Entry: 0x0005AA96
// Name: FUN_0005aa96
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000E34 reset_vdp_upload_state
//   0x00000FFE clear_and_fill_plane_a
//   0x0000102A clear_and_fill_plane_b
//   0x000012F6 advance_title_palette_fade
//   0x00001368 copy_target_palette
//   0x00001530 decode_and_upload_source_stream
//   0x00001728 render_hud_sprite_stream
//   0x0000195C sample_controller_input
//   0x0000C0C2 load_title_font
//   0x0005AA5A render_title_intro_stage_text_batch
//   0x0005AB96 advance_title_intro_stage_service_steps
//   0x0005ABD2 FUN_0005abd2
//   0x0005AC28 reset_title_intro_stage_projection
//   0x0005AC70 reset_title_intro_stage_timed_sprites
//   0x0005B32C FUN_0005b32c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005aa96(void)

{
  undefined2 uVar1;
  short sVar2;
  
  reset_vdp_upload_state();
  clear_and_fill_plane_a();
  clear_and_fill_plane_b();
  decode_and_upload_source_stream();
  decode_and_upload_source_stream();
  load_title_font();
  render_title_intro_stage_text_batch();
  wait_for_vblank();
  copy_target_palette();
  FUN_0005b32c();
  DAT_00fff11e = DAT_00ffdf88;
  reset_title_intro_stage_projection();
  reset_title_intro_stage_timed_sprites();
  FUN_0005abd2();
  wait_for_vblank();
  render_hud_sprite_stream();
  uVar1 = DAT_00ffdf88;
  advance_title_palette_fade();
  DAT_00ffdf88 = uVar1;
  sample_controller_input();
  sVar2 = 0x114;
  do {
    wait_for_vblank();
    sample_controller_input();
    latch_controller_events();
    FUN_0005abd2();
    advance_title_intro_stage_service_steps();
    if (_DAT_00ffddc2 != 0) {
      return;
    }
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


