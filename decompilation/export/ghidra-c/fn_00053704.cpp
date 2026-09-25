// Ghidra headless export
// Entry: 0x00053704
// Name: convert_audio_routine_0x00053704
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x000176EA render_menu_footer
//   0x00017708 render_field_hud
//   0x000177CC upload_menu_resource_pair
//   0x000532BC convert_audio_routine_0x000532bc
//   0x000537FA convert_audio_routine_0x000537fa


undefined4 convert_audio_routine_0x00053704(void)

{
  undefined4 uVar1;
  short sVar2;
  int extraout_A1;
  int iVar3;
  
  convert_audio_routine_0x000532bc();
  convert_audio_routine_0x000537fa();
  do {
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if ((DAT_00ffdd22._0_1_ & 4) == 0) break;
      if (extraout_A1 + -0xff1101 < 0) {
        iVar3 = 0xff1107;
        sVar2 = 6;
        do {
          if (-1 < *(char *)(iVar3 + 7 + (int)(short)(DAT_00fff118 * 0x14))) goto LAB_000537c6;
          iVar3 = iVar3 + -1;
          sVar2 = sVar2 + -1;
        } while (sVar2 != -1);
      }
      else {
LAB_000537c6:
        convert_audio_routine_0x000537fa();
      }
    }
    if ((((DAT_00ffdd22._0_1_ & 0x40) != 0) || ((DAT_00ffdd22._0_1_ & 0x10) != 0)) ||
       ((DAT_00ffdd22._0_1_ & 0x20) != 0)) {
      DAT_00fff11a = DAT_00fff118;
      DAT_00fff118 = (short)*(char *)(extraout_A1 + 7 + (int)(short)(DAT_00fff118 * 0x14));
      if (DAT_00fff0a9 == '\0') {
        register0x0000003c = (BADSPACEBASE *)&stack0x00000004;
      }
      *(undefined4 *)((int)register0x0000003c + -4) = 0x537b4;
      render_field_hud();
      return 0xffffffff;
    }
    if ((DAT_00ffdd22._0_1_ & 8) != 0) goto LAB_000537c6;
    if (((DAT_00ffdd22._0_1_ & 0x80) != 0) && (DAT_00fff0a9 == '\0')) {
      VDP_CTRL = 0x8134;
      upload_menu_resource_pair();
      uVar1 = render_menu_footer();
      return uVar1;
    }
  } while( true );
}


