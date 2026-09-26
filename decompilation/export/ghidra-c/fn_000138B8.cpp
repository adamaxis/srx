// Ghidra headless export
// Entry: 0x000138B8
// Name: run_entity_animation_entry
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00013A92 run_entity_animation_branch
//   0x00014508 convert_entity_routine_0x00014508


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void run_entity_animation_entry(void)

{
  byte bVar1;
  short sVar2;
  int iVar3;
  short sVar4;
  ushort uVar5;
  short sVar6;
  short extraout_D1w;
  short extraout_D1w_00;
  ushort uVar7;
  short sVar8;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  ushort *extraout_A0_04;
  ushort *extraout_A0_05;
  ushort *extraout_A0_06;
  ushort *extraout_A0_07;
  ushort *puVar9;
  int extraout_A0_08;
  ushort *extraout_A0_09;
  int extraout_A0_10;
  ushort *extraout_A0_11;
  int extraout_A0_12;
  int extraout_A0_13;
  int extraout_A0_14;
  int iVar10;
  undefined4 extraout_A1;
  short *extraout_A1_00;
  short *extraout_A1_01;
  short *extraout_A1_02;
  short *psVar11;
  ushort *puVar12;
  bool bVar13;
  undefined1 uVar14;
  ushort uStack_14;
  short sStack_12;
  short sStack_10;
  short sStack_e;
  short sStack_c;
  undefined1 auStack_a [2];
  undefined4 uStack_8;
  undefined4 uStack_4;
  
  sVar8 = *(short *)(in_A0 + 0xb6) + -1;
  *(short *)(in_A0 + 0xb6) = sVar8;
  if (-1 < sVar8) {
    return;
  }
  if (DAT_00ffe175 != '\0') {
    return;
  }
  DAT_00ffe175 = 0xff;
  *(undefined2 *)(in_A0 + 0xb6) = 0x14;
  if ((*(char *)(in_A0 + 0x4d) == '\0') || (_DAT_00ffdf84 == 0)) {
    uStack_4 = 0x138f2;
    convert_entity_routine_0x00014508();
    return;
  }
  if (*(short *)(in_A0 + 0xdc) < 0) {
    uStack_4 = 0x13900;
    run_entity_animation_branch();
    in_A0 = extraout_A0;
  }
  if (*(int *)(in_A0 + 0xac) == 0) {
    uStack_4 = 0x139e4;
    iVar3 = dispatch_entity_animation_state();
    iVar10 = extraout_A0_00;
  }
  else {
    uStack_4 = 0x13a0a;
    measure_relative_heading_and_distance();
    uStack_8 = 0x13a10;
    uStack_4 = extraout_A1;
    iVar3 = dispatch_entity_animation_state();
    iVar10 = extraout_A0_01;
  }
  if (iVar3 != 0x12345678) {
    convert_menu_routine_0x00013d0c();
    return;
  }
  iVar3 = DAT_00fffc04;
  if (*(int *)(iVar10 + 0xa2) != 0) {
    iVar3 = *(int *)(iVar10 + 0xa2);
  }
  sVar8 = *(short *)(iVar10 + 0x3e);
  if ((sVar8 == 0) || ((sVar8 == 1 && (*(short *)(iVar3 + 0x3e) == 0)))) {
    sVar6 = *(short *)(iVar3 + 0x2e) + -0x6000;
  }
  else {
    sVar6 = *(short *)(iVar3 + 0x2e) + 0x6000;
  }
  uStack_4 = 0x14540;
  convert_entity_routine_0x00015dcc();
  uStack_4 = CONCAT22(sVar6 + extraout_A1_00[2],uStack_4._2_2_);
  uStack_8 = CONCAT22(*extraout_A1_00 + 0xe0,uStack_8._2_2_);
  uStack_14 = 0xe0;
  sStack_10 = sVar6;
  sStack_c = sVar8;
  sVar4 = evaluate_terrain_collision();
  sVar2 = sStack_10;
  puVar12 = (ushort *)auStack_a;
  iVar10 = extraout_A0_02;
  uVar7 = uStack_14;
  sVar8 = sStack_e;
  sVar6 = sStack_c;
  if ((sVar4 != 0) && (-1 < sVar4)) {
    uVar7 = -uStack_14;
    sStack_e = extraout_A1_01[2] - sStack_12;
    sStack_12 = uVar7 + *extraout_A1_01;
    sVar4 = evaluate_terrain_collision();
    sVar8 = 1;
    puVar12 = &uStack_14;
    iVar10 = extraout_A0_03;
    sVar6 = sVar2;
    if ((sVar4 != 0) && (-1 < sVar4)) {
      sVar8 = *extraout_A1_02;
      sVar6 = extraout_A1_02[2];
    }
  }
  *(short *)(iVar10 + 0x96) = sVar8;
  *(short *)(iVar10 + 0x98) = sVar6;
  bVar13 = false;
  puVar12[-0xffffffff00000002] = 1;
  puVar12[-0xffffffff00000001] = 0x459c;
  apply_entity_close_range_state();
  if (bVar13) {
    return;
  }
  uVar14 = extraout_A0_04[0xb] < 0x3c;
  if (0x3c < (short)extraout_A0_04[0xb]) {
    return;
  }
  puVar9 = extraout_A0_04;
  if ((extraout_A0_04[0x58] & 0x200) == 0) {
    bVar1 = *(byte *)(extraout_A0_04 + 0x58);
    *(byte *)(extraout_A0_04 + 0x58) = bVar1 & 0xfb;
    if ((bVar1 & 4) == 0) {
      uVar14 = extraout_A0_04[0x5c] < 0x1000;
      if (extraout_A0_04[0x5c] != 0x1000) {
        puVar12[-0xffffffff00000002] = 1;
        puVar12[-0xffffffff00000001] = 0x45da;
        update_entity_axis_relation_state();
        puVar9 = extraout_A0_06;
        if ((extraout_A0_06[0x58] & 0x200) != 0) goto advance_obstruction_route_queue;
        puVar12[-0xffffffff00000002] = 1;
        puVar12[-0xffffffff00000001] = 0x45e8;
        derive_entity_axis_relation_flags();
        puVar9 = extraout_A0_07;
      }
    }
    else {
      puVar12[-0xffffffff00000002] = 1;
      puVar12[-0xffffffff00000001] = 0x45c8;
      derive_entity_axis_relation_flags();
      puVar9 = extraout_A0_05;
    }
LAB_000145e8:
    uVar5 = puVar9[0x4b];
    uVar7 = puVar9[0x4c];
    puVar12[-0xffffffff00000002] = 1;
    puVar12[-0xffffffff00000001] = 0x45fc;
    measure_relative_heading_and_distance();
    iVar10 = extraout_A0_08;
  }
  else {
advance_obstruction_route_queue:
    if ((puVar9[0x58] & 0x800) != 0) {
      puVar12[-0xffffffff00000002] = 1;
      puVar12[-0xffffffff00000001] = 0x49ce;
      is_entity_path_blocked();
      puVar9 = extraout_A0_09;
      if ((bool)uVar14) {
        *(byte *)(extraout_A0_09 + 0x58) = *(byte *)(extraout_A0_09 + 0x58) & 0xfd;
        *(byte *)(extraout_A0_09 + 0x58) = *(byte *)(extraout_A0_09 + 0x58) & 0xf7;
        return;
      }
    }
    sVar8 = (puVar9[2] >> 4 & 0x7fe0) * 2 + (*puVar9 >> 9);
    psVar11 = (short *)((int)puVar9 + (short)puVar9[0x5d] + 0xbc);
    if (sVar8 == *psVar11) {
      if (psVar11[1] == -1) {
        if ((puVar9[0x58] & 0x800) != 0) {
          puVar12[-0xffffffff00000002] = 1;
          puVar12[-0xffffffff00000001] = 0x4a64;
          compute_obstruction_grid_index();
          uVar7 = *(ushort *)(extraout_A0_12 + 0x34) >> 0xd & 6;
          if (((&DAT_00ff1700)[(short)(*(short *)(&DAT_000148ca + (short)uVar7) + sVar8)] == '\0')
             && ((&DAT_00ff1700)
                 [(short)(*(short *)(&DAT_000148ca + (short)(uVar7 + 2 & 6)) +
                         *(short *)(&DAT_000148ca + (short)(uVar7 + 4 & 6)) + sVar8)] == '\0')) {
            puVar12[-0xffffffff00000002] = 1;
            puVar12[-0xffffffff00000001] = 0x4ab0;
            find_first_occupied_obstruction_tile();
            sVar6 = extraout_D1w;
            iVar10 = extraout_A0_13;
          }
          else {
            sVar6 = 4;
            while( true ) {
              puVar12[-1] = sVar6;
              puVar12[-0xffffffff00000003] = 1;
              puVar12[-0xffffffff00000002] = 0x4ac6;
              find_alternate_route_tile();
              sVar6 = extraout_D1w_00;
              iVar10 = extraout_A0_14;
              if (extraout_D1w_00 != sVar8) break;
              sVar6 = puVar12[-1] + -1;
              if (sVar6 == -1) {
                clear_entity_route_flags();
                return;
              }
            }
          }
          *(undefined2 *)(iVar10 + 0xba) = 0;
          *(short *)(iVar10 + 0xbc) = sVar6;
          *(undefined2 *)(iVar10 + 0xbe) = 0xffff;
          return;
        }
        *(byte *)(puVar9 + 0x58) = *(byte *)(puVar9 + 0x58) & 0xfd;
        *(byte *)(puVar9 + 0x58) = *(byte *)(puVar9 + 0x58) & 0xf7;
        puVar12[-0xffffffff00000002] = 1;
        puVar12[-0xffffffff00000001] = 0x4a50;
        derive_entity_axis_relation_flags();
        puVar9 = extraout_A0_11;
        goto LAB_000145e8;
      }
      puVar9[0x5d] = puVar9[0x5d] + 2;
    }
    puVar12[-0xffffffff00000002] = 1;
    puVar12[-0xffffffff00000001] = 0x4a12;
    decode_obstruction_tile_projection();
    puVar12[-1] = uVar7 << 0xd | uVar7 >> 3;
    puVar12[-0xffffffff00000003] = 1;
    puVar12[-0xffffffff00000002] = 0x4a2e;
    uVar5 = compute_diagonal_step_cost();
    uVar7 = puVar12[-1];
    iVar10 = extraout_A0_10;
  }
  *(ushort *)(iVar10 + 0x34) = uVar7;
  *(ushort *)(iVar10 + 200) = uVar5;
  *(undefined1 **)(iVar10 + 0x3a) = &LAB_0000b574;
  return;
}

