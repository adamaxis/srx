// Ghidra headless export
// Entry: 0x000134D2
// Name: FUN_000134d2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00001530 decode_and_upload_source_stream
//   0x00001728 render_hud_sprite_stream
//   0x0000195C sample_controller_input
//   0x0000391A FUN_0000391a
//   0x0000E6F4 FUN_0000e6f4
//   0x00010AF0 FUN_00010af0
//   0x000136DE initialize_scene_mode_state
//   0x00013726 FUN_00013726
//   0x00020714 FUN_00020714
//   0x00053126 prepare_mode_entry_service


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000134d2(void)

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
    FUN_0000e6f4();
  }
  if (DAT_00ffe18b != '\0') {
    DAT_00fff09c = 4;
    FUN_00013726();
    sVar2 = 0x40;
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if (DAT_00fff09e == '\0') {
        FUN_0000e6f4();
      }
      if (_DAT_00ffddc2 != 0) break;
      sVar2 = sVar2 + -1;
      if (sVar2 == -1) {
        DAT_00fff09c = 2;
        FUN_00013726();
        sVar2 = 0x60;
        do {
          wait_for_vblank();
          latch_controller_events();
          sample_controller_input();
          if (DAT_00fff09e == '\0') {
            FUN_0000e6f4();
          }
        } while ((_DAT_00ffddc2 == 0) && (sVar2 = sVar2 + -1, sVar2 != -1));
        DAT_00fff09e = 0;
        return;
      }
    }
    DAT_00fff09e = 0;
    return;
  }
  FUN_00013726();
  while( true ) {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    if (DAT_00fff09e == '\0') {
      FUN_0000e6f4();
    }
    if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 8) != 0)) {
      DAT_00fff09c = DAT_00fff09c + -1;
      if (DAT_00fff09c < '\0') {
        DAT_00fff09c = '\x06';
      }
      FUN_00013726();
    }
    if (((DAT_00ffdd22._0_1_ & 4) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) {
      cVar1 = DAT_00fff09c + -6;
      DAT_00fff09c = DAT_00fff09c + '\x01';
      if (-1 < cVar1) {
        DAT_00fff09c = '\0';
      }
      FUN_00013726();
    }
    if (((DAT_00ffdd22._0_1_ & 0x40) != 0) || ((DAT_00ffdd22._0_1_ & 0x10) != 0)) break;
    if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
      FUN_0000391a();
    }
    if ((DAT_00ffdd22._0_1_ & 0x80) != 0) {
      FUN_00010af0();
LAB_000136b6:
      if ((DAT_00fff0e0 == '\0') && ((char)DAT_00fffbf8 != DAT_00ffddc4)) {
        initialize_scene_mode_state();
        prepare_mode_entry_service();
      }
      return;
    }
  }
  FUN_00020714();
  DAT_00fff0dc = 0xff;
  goto LAB_000136b6;
}


