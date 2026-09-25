// Ghidra headless export
// Entry: 0x00026A00
// Name: convert_world_routine_0x00026a00
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000F5C write_vdp_word_block_with_source_wrap
//   0x00001358 copy_source_word_block
//   0x0000195C sample_controller_input
//   0x00010952 reset_mode_display_buffers
//   0x00026BFE write_masked_transfer
//   0x00026C26 write_masked_transfer_with_display_control
//   0x000530DE service_audio_if_enabled


void convert_world_routine_0x00026a00(void)

{
  short extraout_D1w;
  short sVar1;
  short extraout_D1w_00;
  undefined4 uVar2;
  uint unaff_D3;
  uint uVar3;
  int iVar4;
  int iVar5;
  undefined **ppuVar6;
  int iVar7;
  undefined2 uVar8;
  
  DAT_00fff0b1 = 0xff;
  service_audio_if_enabled();
  write_vdp_word_block_with_source_wrap();
  reset_mode_display_buffers();
  copy_source_word_block();
  DAT_00ffef14 = DAT_00ffef14 | 1;
  uVar2 = 0x6000;
  write_masked_transfer();
  write_masked_transfer_with_display_control();
  uVar3 = unaff_D3 & 0xffff0000;
  write_masked_transfer();
  sVar1 = 0xd;
  iVar4 = 0xe6fe;
  iVar5 = 0xe752;
  ppuVar6 = &PTR_DAT_0000dfa8;
  iVar7 = 0xeea8;
  do {
    wait_for_vblank(sVar1,uVar2,uVar3,iVar4,iVar5,ppuVar6,iVar7);
    uVar8 = (undefined2)((uint)uVar2 >> 0x10);
    latch_controller_events();
    sample_controller_input();
    iVar4 = iVar4 + 2;
    uVar3 = uVar3 & 0xffff0000;
    uVar2 = CONCAT22(uVar8,0x6000);
    write_masked_transfer();
    iVar5 = iVar5 + -2;
    write_masked_transfer();
    ppuVar6 = ppuVar6 + 0x20;
    write_masked_transfer_with_display_control();
    iVar7 = iVar7 + -0x80;
    write_masked_transfer_with_display_control();
    sVar1 = extraout_D1w + -1;
  } while (sVar1 != -1);
  sVar1 = 5;
  do {
    wait_for_vblank(sVar1,uVar2,uVar3,iVar4,iVar5);
    uVar8 = (undefined2)((uint)uVar2 >> 0x10);
    latch_controller_events();
    sample_controller_input();
    iVar4 = iVar4 + 2;
    uVar3 = uVar3 & 0xffff0000;
    uVar2 = CONCAT22(uVar8,0x6000);
    write_masked_transfer();
    iVar5 = iVar5 + -2;
    write_masked_transfer();
    sVar1 = extraout_D1w_00 + -1;
  } while (sVar1 != -1);
  write_masked_transfer();
  sVar1 = 10;
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  write_masked_transfer();
  sVar1 = 5;
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  write_masked_transfer();
  sVar1 = 5;
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  write_masked_transfer();
  sVar1 = 10;
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


