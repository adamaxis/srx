// Ghidra headless export
// Entry: 0x00013D0C
// Name: run_entity_animation_motion
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00013A40 copy_entity_anchor_position
//   0x000144E8 apply_entity_close_range_state
//   0x000145A2 convert_entity_routine_0x000145a2
//   0x00014D6A select_entity_animation_table_row
//   0x00015D3C measure_relative_heading_and_distance
//   0x00015DCC convert_entity_routine_0x00015dcc


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort * run_entity_animation_motion(void)

{
  byte bVar1;
  short sVar2;
  ushort *in_D0;
  ushort *puVar3;
  ushort *puVar4;
  ushort uVar5;
  short extraout_D1w;
  short extraout_D1w_00;
  short sVar6;
  ushort uVar7;
  ushort *in_A0;
  ushort *extraout_A0;
  ushort *extraout_A0_00;
  short *extraout_A0_01;
  ushort *extraout_A0_02;
  ushort *extraout_A0_03;
  ushort *extraout_A0_04;
  ushort *extraout_A0_05;
  int extraout_A0_06;
  ushort *extraout_A0_07;
  int extraout_A0_08;
  ushort *extraout_A0_09;
  int extraout_A0_10;
  int extraout_A0_11;
  int extraout_A0_12;
  int iVar8;
  ushort *extraout_A1;
  ushort *extraout_A1_00;
  int unaff_A2;
  short *psVar9;
  undefined *puVar10;
  bool bVar11;
  undefined1 uVar12;
  
  if (((DAT_00ffe186 != '\0') && ((in_A0[8] & 0x800) == 0)) && (*(char *)(in_A0 + 0x27) == '('))
  goto LAB_00013e40;
  if (DAT_00ffdfe0 != 0) {
    if (DAT_00ffdfe0 != 2) {
      return in_D0;
    }
    if (*(ushort **)(in_A0 + 0x51) == (ushort *)0x0) {
      return *(ushort **)(in_A0 + 0x51);
    }
    copy_entity_anchor_position();
    puVar3 = (ushort *)convert_entity_routine_0x000145a2();
    return puVar3;
  }
  puVar3 = *(ushort **)(in_A0 + 0x51);
  if (puVar3 == (ushort *)0x0) {
    return puVar3;
  }
  uVar5 = *puVar3;
  uVar7 = puVar3[2];
  measure_relative_heading_and_distance();
  *(byte *)(extraout_A0 + 0x58) = *(byte *)(extraout_A0 + 0x58) & 0xef;
  in_A0 = extraout_A0;
  if ((extraout_A0[8] & 0x800) == 0) {
    if (*(char *)((int)extraout_A0 + 0xb5) != '\0') {
      *(char *)((int)extraout_A0 + 0xb5) = *(char *)((int)extraout_A0 + 0xb5) + -1;
      select_entity_animation_table_row();
      bVar1 = *(byte *)(unaff_A2 + 0x27);
      puVar10 = &DAT_00013e7c;
      in_A0 = extraout_A0_00;
      puVar3 = extraout_A1_00;
      goto LAB_00013db0;
    }
LAB_00013dca:
    *(byte *)(in_A0 + 0x58) = *(byte *)(in_A0 + 0x58) | 0x10;
    if (_DAT_00ffdfde != 0) goto LAB_00013e40;
    convert_entity_routine_0x00015dcc();
    uVar7 = extraout_A0_01[2] + uVar7 + 0x4000;
    extraout_A0_01[0x4b] = *extraout_A0_01 + uVar5 * 2;
    extraout_A0_01[0x4c] = uVar7;
  }
  else {
    puVar10 = &DAT_00013e58;
    bVar1 = *(byte *)((int)extraout_A0 + 0x4d);
    puVar3 = extraout_A1;
LAB_00013db0:
    if (uVar5 < *(ushort *)(puVar10 + (short)((ushort)bVar1 << 2))) goto LAB_00013dca;
    if (uVar5 < *(ushort *)(puVar10 + (short)((ushort)bVar1 << 2) + 2)) {
      *(undefined1 *)((int)in_A0 + 0xb5) = 0;
      goto LAB_00013dca;
    }
    in_A0[0x4b] = *puVar3;
    in_A0[0x4c] = puVar3[2];
  }
  bVar11 = false;
  apply_entity_close_range_state();
  if (bVar11) {
    *(undefined1 *)((int)extraout_A0_02 + 0xb5) = 0;
    in_A0 = extraout_A0_02;
LAB_00013e40:
    uVar5 = in_A0[0xb];
    if (0x3c < (short)uVar5) {
      return (ushort *)(uint)uVar5;
    }
    in_A0[0x1d] = 0;
    in_A0[0x1e] = 0xb45c;
    return (ushort *)(uint)uVar5;
  }
  uVar5 = extraout_A0_02[0xb];
  uVar12 = uVar5 < 0x3c;
  if (0x3c < (short)uVar5) {
    return (ushort *)(uint)uVar5;
  }
  puVar3 = extraout_A0_02;
  if ((extraout_A0_02[0x58] & 0x200) == 0) {
    bVar1 = *(byte *)(extraout_A0_02 + 0x58);
    *(byte *)(extraout_A0_02 + 0x58) = bVar1 & 0xfb;
    if ((bVar1 & 4) == 0) {
      uVar12 = extraout_A0_02[0x5c] < 0x1000;
      if (extraout_A0_02[0x5c] != 0x1000) {
        update_entity_axis_relation_state();
        puVar3 = extraout_A0_04;
        if ((extraout_A0_04[0x58] & 0x200) != 0) goto advance_obstruction_route_queue;
        derive_entity_axis_relation_flags();
        puVar3 = extraout_A0_05;
      }
    }
    else {
      derive_entity_axis_relation_flags();
      puVar3 = extraout_A0_03;
    }
LAB_000145e8:
    uVar5 = puVar3[0x4b];
    uVar7 = puVar3[0x4c];
    puVar3 = (ushort *)measure_relative_heading_and_distance();
    iVar8 = extraout_A0_06;
  }
  else {
advance_obstruction_route_queue:
    if (((puVar3[0x58] & 0x800) != 0) &&
       (puVar4 = (ushort *)is_entity_path_blocked(), puVar3 = extraout_A0_07, (bool)uVar12)) {
      *(byte *)(extraout_A0_07 + 0x58) = *(byte *)(extraout_A0_07 + 0x58) & 0xfd;
      *(byte *)(extraout_A0_07 + 0x58) = *(byte *)(extraout_A0_07 + 0x58) & 0xf7;
      return puVar4;
    }
    sVar2 = (puVar3[2] >> 4 & 0x7fe0) * 2 + (*puVar3 >> 9);
    psVar9 = (short *)((int)puVar3 + (short)puVar3[0x5d] + 0xbc);
    if (sVar2 == *psVar9) {
      if (psVar9[1] == -1) {
        if ((puVar3[0x58] & 0x800) != 0) {
          compute_obstruction_grid_index();
          uVar5 = *(ushort *)(extraout_A0_10 + 0x34) >> 0xd & 6;
          if (((&DAT_00ff1700)[(short)(*(short *)(&DAT_000148ca + (short)uVar5) + sVar2)] == '\0')
             && ((&DAT_00ff1700)
                 [(short)(*(short *)(&DAT_000148ca + (short)(uVar5 + 2 & 6)) +
                         *(short *)(&DAT_000148ca + (short)(uVar5 + 4 & 6)) + sVar2)] == '\0')) {
            puVar3 = (ushort *)find_first_occupied_obstruction_tile();
            sVar6 = extraout_D1w;
            iVar8 = extraout_A0_11;
          }
          else {
            uVar5 = 4;
            while( true ) {
              find_alternate_route_tile();
              puVar3 = (ushort *)(uint)uVar5;
              sVar6 = extraout_D1w_00;
              iVar8 = extraout_A0_12;
              if (extraout_D1w_00 != sVar2) break;
              uVar5 = uVar5 - 1;
              if (uVar5 == 0xffff) {
                puVar3 = (ushort *)clear_entity_route_flags();
                return puVar3;
              }
            }
          }
          *(undefined2 *)(iVar8 + 0xba) = 0;
          *(short *)(iVar8 + 0xbc) = sVar6;
          *(undefined2 *)(iVar8 + 0xbe) = 0xffff;
          return puVar3;
        }
        *(byte *)(puVar3 + 0x58) = *(byte *)(puVar3 + 0x58) & 0xfd;
        *(byte *)(puVar3 + 0x58) = *(byte *)(puVar3 + 0x58) & 0xf7;
        derive_entity_axis_relation_flags();
        puVar3 = extraout_A0_09;
        goto LAB_000145e8;
      }
      puVar3[0x5d] = puVar3[0x5d] + 2;
    }
    decode_obstruction_tile_projection();
    uVar7 = uVar7 << 0xd | uVar7 >> 3;
    puVar3 = (ushort *)compute_diagonal_step_cost();
    uVar5 = (ushort)puVar3;
    iVar8 = extraout_A0_08;
  }
  *(ushort *)(iVar8 + 0x34) = uVar7;
  *(ushort *)(iVar8 + 200) = uVar5;
  *(undefined1 **)(iVar8 + 0x3a) = &LAB_0000b574;
  return puVar3;
}

