// Ghidra headless export
// Entry: 0x000119CE
// Name: run_command_overview_action_dialog
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00001232 write_vdp_grid
//   0x0000195C sample_controller_input
//   0x00011ABE prepare_menu_dialog
//   0x000530DE service_audio_if_enabled


undefined8 run_command_overview_action_dialog(void)

{
  undefined4 uVar1;
  undefined4 extraout_D1;
  undefined4 extraout_D1_00;
  uint unaff_D2;
  uint uVar2;
  undefined4 unaff_D3;
  undefined *puVar3;
  undefined4 unaff_D5;
  undefined4 unaff_D6;
  undefined4 uVar4;
  undefined4 extraout_A0;
  undefined4 uVar5;
  undefined4 extraout_A0_00;
  undefined4 extraout_A0_01;
  undefined4 extraout_A1;
  undefined4 uVar6;
  undefined4 extraout_A1_00;
  undefined4 extraout_A1_01;
  undefined4 unaff_A2;
  undefined4 unaff_A3;
  undefined4 unaff_A4;
  undefined4 unaff_A5;
  undefined4 unaff_A6;
  undefined8 uVar7;
  
  puVar3 = &UNK_0000ca9e;
  uVar4 = 5;
  uVar2 = unaff_D2 & 0xffff0000;
  write_vdp_grid();
  prepare_menu_dialog();
  uVar1 = extraout_D1;
  uVar5 = extraout_A0;
  uVar6 = extraout_A1;
  do {
    while( true ) {
      wait_for_vblank(uVar1,uVar2,unaff_D3,puVar3,unaff_D5,unaff_D6,uVar4,uVar5,uVar6,unaff_A2,
                      unaff_A3,unaff_A4,unaff_A5,unaff_A6);
      latch_controller_events();
      sample_controller_input();
      if (((DAT_00ffdd22 & 0x200) != 0) && ((DAT_00ffdd22 & 8) == 0)) break;
LAB_00011a44:
      if ((DAT_00ffdd22 & 0x800) == 0) goto LAB_00011a5c;
      uVar1 = 1;
LAB_00011a70:
      service_audio_if_enabled(uVar1);
      prepare_menu_dialog();
      uVar1 = extraout_D1_00;
      uVar5 = extraout_A0_00;
      uVar6 = extraout_A1_00;
    }
    if ((DAT_00ffdd22 & 4) == 0) {
      if (-1 < DAT_00fff115) goto LAB_00011a44;
      goto LAB_00011a9a;
    }
LAB_00011a5c:
    if ((DAT_00ffdd22 & 0x400) != 0) {
      uVar1 = 0xffffffff;
      goto LAB_00011a70;
    }
  } while ((DAT_00ffdd22 & 0x8000) == 0);
  DAT_00fff0b1 = 0xff;
LAB_00011a9a:
  write_vdp_grid();
  uVar7 = adjust_menu_dialog_posture();
  update_equipment_combat_bars
            (uVar7,uVar2 & 0xffff0000,unaff_D3,&UNK_0000ca9e,unaff_D5,unaff_D6,5,extraout_A0_01,
             extraout_A1_01,unaff_A2,unaff_A3,unaff_A4,unaff_A5,unaff_A6);
  update_equipment_combat_bars();
  return uVar7;
}


