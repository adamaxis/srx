// Ghidra headless export
// Entry: 0x00013726
// Name: render_mode_selection
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000E9C make_vdp_transfer_command
//   0x0000C0DA render_title_text_source
//   0x0000C3E6 write_menu_grid_with_metadata
//   0x0000E6F4 service_mode_world_state


void render_mode_selection(void)

{
  word wVar1;
  byte bVar2;
  undefined2 extraout_D0u;
  undefined4 uVar3;
  short extraout_D1w;
  undefined4 extraout_D1;
  uint uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  undefined4 unaff_D7;
  short sVar8;
  ushort uVar9;
  ushort uVar10;
  ushort *extraout_A0;
  undefined4 extraout_A0_00;
  word *extraout_A1;
  word *extraout_A1_00;
  word *extraout_A1_01;
  word *extraout_A1_02;
  int extraout_A1_03;
  word *pwVar11;
  undefined1 *puVar12;
  byte *pbVar13;
  undefined4 unaff_A4;
  undefined2 unaff_A5w;
  undefined2 unaff_A6w;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  word *pwVar17;
  
  pwVar11 = &VDP_DATA;
  puVar12 = &DAT_000d4536;
  uVar7 = 4;
  do {
    write_menu_grid_with_metadata();
    uVar6 = (uint)*extraout_A0;
    pbVar13 = puVar12 + 1;
    puVar12 = puVar12 + 2;
    bVar2 = *pbVar13;
    uVar4 = (uint)bVar2;
    make_vdp_transfer_command();
    *pwVar11 = *extraout_A1;
    wVar1 = extraout_A1[1];
    sVar8 = extraout_D1w;
    do {
      *pwVar11 = wVar1;
      sVar8 = sVar8 + -1;
    } while (sVar8 != -1);
    *pwVar11 = extraout_A1[2];
    VDP_CTRL = 0x8f80;
    make_vdp_transfer_command();
    wVar1 = *extraout_A1_00;
    uVar10 = (ushort)bVar2;
    uVar9 = uVar10;
    do {
      *pwVar11 = wVar1;
      uVar9 = uVar9 - 1;
    } while (uVar9 != 0xffff);
    make_vdp_transfer_command();
    wVar1 = *extraout_A1_01;
    do {
      *pwVar11 = wVar1;
      uVar10 = uVar10 - 1;
    } while (uVar10 != 0xffff);
    VDP_CTRL = 0x8f02;
    iVar5 = (uVar4 + 2) * 0x80;
    make_vdp_transfer_command();
    *pwVar11 = *extraout_A1_02;
    wVar1 = extraout_A1_02[1];
    unaff_D7 = CONCAT22((short)((uint)unaff_D7 >> 0x10),(short)extraout_D1);
    do {
      *pwVar11 = wVar1;
      sVar8 = (short)unaff_D7 + -1;
      unaff_D7 = CONCAT22((short)((uint)unaff_D7 >> 0x10),sVar8);
    } while (sVar8 != -1);
    pwVar17 = extraout_A1_02 + 3;
    uVar3 = CONCAT22(extraout_D0u,extraout_A1_02[2]);
    *pwVar11 = extraout_A1_02[2];
    uVar15 = 0xc208;
    sVar8 = 1;
    uVar14 = extraout_D1;
    uVar16 = extraout_A0_00;
    do {
      wait_for_vblank(uVar3,uVar14,uVar4,iVar5,uVar15,uVar6,uVar7,unaff_D7,uVar16,pwVar17,pwVar11,
                      puVar12,unaff_A4,unaff_A5w,unaff_A6w);
      latch_controller_events();
      if (DAT_00fff09e == '\0') {
        service_mode_world_state();
      }
      sVar8 = sVar8 + -1;
    } while (sVar8 != -1);
    uVar9 = (short)uVar7 - 1;
    uVar7 = (uint)uVar9;
  } while (uVar9 != 0xffff);
  render_title_text_source();
  DAT_00fff0e0 = *(undefined1 *)(extraout_A1_03 + 0x13);
  return;
}


