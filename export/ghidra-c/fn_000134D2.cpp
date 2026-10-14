// Ghidra headless export
// Entry: 0x000134D2
// Name: run_post_intro_mode
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00001530 decode_and_upload_source_stream
//   0x00001728 render_hud_sprite_stream
//   0x0000195C sample_controller_input
//   0x0000391A handle_mode_action_event
//   0x0000E6F4 service_mode_world_state
//   0x00010AF0 transfer_menu_asset_batch_four
//   0x000136DE initialize_scene_mode_state
//   0x00013726 render_mode_selection
//   0x00020714 finalize_mode_exit
//   0x00053126 prepare_mode_entry_service


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void run_post_intro_mode(void)

{
  char cVar1;
  short sVar2;
  
  DAT_00fff09c = (&DAT_000d4456)[(short)(ushort)DAT_00fffbf8];
  decode_and_upload_source_stream();
  render_hud_sprite_stream();
  wait_for_vblank();
  latch_controller_events();
  sample_controller_input();
  if (DAT_00fff09e == '\0') {
    service_mode_world_state();
  }
  if (DAT_00ffe18b != '\0') {
    DAT_00fff09c = 4;
    render_mode_selection();
    sVar2 = 0x40;
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if (DAT_00fff09e == '\0') {
        service_mode_world_state();
      }
      if (_DAT_00ffddc2 != 0) break;
      sVar2 = sVar2 + -1;
      if (sVar2 == -1) {
        DAT_00fff09c = 2;
        render_mode_selection();
        sVar2 = 0x60;
        do {
          wait_for_vblank();
          latch_controller_events();
          sample_controller_input();
          if (DAT_00fff09e == '\0') {
            service_mode_world_state();
          }
        } while ((_DAT_00ffddc2 == 0) && (sVar2 = sVar2 + -1, sVar2 != -1));
        DAT_00fff09e = 0;
        return;
      }
    }
    DAT_00fff09e = 0;
    return;
  }
  render_mode_selection();
  while( true ) {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    if (DAT_00fff09e == '\0') {
      service_mode_world_state();
    }
    if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 8) != 0)) {
      DAT_00fff09c = DAT_00fff09c + -1;
      if (DAT_00fff09c < '\0') {
        DAT_00fff09c = '\x06';
      }
      render_mode_selection();
    }
    if (((DAT_00ffdd22._0_1_ & 4) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) {
      cVar1 = DAT_00fff09c + -6;
      DAT_00fff09c = DAT_00fff09c + '\x01';
      if (-1 < cVar1) {
        DAT_00fff09c = '\0';
      }
      render_mode_selection();
    }
    if (((DAT_00ffdd22._0_1_ & 0x40) != 0) || ((DAT_00ffdd22._0_1_ & 0x10) != 0)) break;
    if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
      handle_mode_action_event();
    }
    if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
      transfer_menu_asset_batch_four();
LAB_000136b6:
      if ((DAT_00fff0e0 == '\0') && ((char)DAT_00fffbf8 != DAT_00ffddc4)) {
        initialize_scene_mode_state();
        prepare_mode_entry_service();
      }
      return;
    }
  }
  finalize_mode_exit();
  DAT_00fff0dc = 0xff;
  goto LAB_000136b6;
}


