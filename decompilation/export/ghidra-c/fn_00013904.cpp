// Ghidra headless export
// Entry: 0x00013904
// Name: FUN_00013904
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000AE6E FUN_0000ae6e
//   0x0000AF7C prepare_entity_delayed_animation_primary
//   0x0000AFAE prepare_entity_delayed_animation_alternate
//   0x00013D0C FUN_00013d0c
//   0x00013E8C FUN_00013e8c
//   0x00013FFE FUN_00013ffe
//   0x00014492 FUN_00014492
//   0x00014508 FUN_00014508
//   0x00015D3C measure_relative_heading_and_distance
//   0x0001DBAA FUN_0001dbaa
//   0x0001E514 FUN_0001e514


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00013904(void)

{
  byte bVar1;
  int iVar2;
  ushort uVar3;
  short sVar4;
  ushort uVar5;
  short extraout_D1w;
  short extraout_D1w_00;
  ushort unaff_D2w;
  short sVar6;
  ushort *in_A0;
  ushort *extraout_A0;
  ushort *extraout_A0_00;
  ushort *extraout_A0_01;
  ushort *extraout_A0_02;
  ushort *extraout_A0_03;
  ushort *extraout_A0_04;
  ushort *extraout_A0_05;
  ushort *extraout_A0_06;
  int extraout_A0_07;
  int extraout_A0_08;
  ushort *extraout_A0_09;
  ushort *extraout_A0_10;
  ushort *extraout_A0_11;
  ushort *extraout_A0_12;
  ushort *puVar7;
  int extraout_A0_13;
  ushort *extraout_A0_14;
  int extraout_A0_15;
  ushort *extraout_A0_16;
  int extraout_A0_17;
  int extraout_A0_18;
  int extraout_A0_19;
  undefined4 extraout_A1;
  ushort *extraout_A1_00;
  short *extraout_A1_01;
  short *extraout_A1_02;
  short *extraout_A1_03;
  short *psVar8;
  ushort *puVar9;
  bool bVar10;
  undefined1 uVar11;
  ushort uStack_14;
  short sStack_12;
  ushort uStack_10;
  short sStack_e;
  ushort uStack_c;
  undefined1 auStack_a [2];
  undefined4 uStack_8;
  undefined4 local_4;
  
  uVar3 = in_A0[0x5b] - 1;
  in_A0[0x5b] = uVar3;
  if (-1 < (short)uVar3) {
    return;
  }
  if (DAT_00ffe175 != '\0') {
    return;
  }
  in_A0[0x5b] = 0x14;
  if (in_A0[7] == 0xf) {
    if (((short)in_A0[0xb] < 0x3d) &&
       (DAT_00ffe188 = DAT_00ffe188 - 1, (short)((ushort)DAT_00ffe188 << 8) < 0)) {
      DAT_00ffe188 = 8;
      local_4 = 0x13a72;
      FUN_00014492();
      local_4 = 0x13a78;
      FUN_0000ae6e();
      if (DAT_00fff0dd == '\0') {
        prepare_entity_delayed_animation_primary();
        return;
      }
      prepare_entity_delayed_animation_alternate();
      return;
    }
LAB_00013e40:
    if ((short)in_A0[0xb] < 0x3d) {
      in_A0[0x1d] = 0;
      in_A0[0x1e] = 0xb45c;
      return;
    }
    return;
  }
  if ((in_A0[0x58] & 0x8000) != 0) {
    FUN_00014508();
    return;
  }
  if ((in_A0[0x4f] & 0x800) != 0) {
    return;
  }
  if (((in_A0[0x4f] & 0x2000) != 0) && (_DAT_00ffdf84 != 0)) {
    FUN_00013e8c();
    return;
  }
  if ((in_A0[6] == 0) && ((in_A0[0x4f] & 0x400) != 0)) {
    FUN_0001e514();
    return;
  }
  if ((in_A0[0x58] & 0x4000) != 0) {
    if (in_A0[6] != 0) {
      local_4 = 0x13e9a;
      FUN_00013e9e();
      FUN_000145a2();
      return;
    }
    goto LAB_00013e40;
  }
  if (in_A0[6] == 0) {
    bVar10 = false;
    if (_DAT_00ffdf84 == 0) goto LAB_000142ae;
    puVar7 = DAT_00fffc04;
    if (((DAT_00ffe186 != '\0') && ((in_A0[8] & 0x800) == 0)) && (*(char *)(in_A0 + 0x27) == '('))
    goto LAB_00013e40;
LAB_0001450e:
    uVar3 = in_A0[0x1f];
    if ((uVar3 == 0) || ((uVar3 == 1 && (puVar7[0x1f] == 0)))) {
      uVar5 = puVar7[0x17] + 0xa000;
    }
    else {
      uVar5 = puVar7[0x17] + 0x6000;
    }
    local_4 = 0x14540;
    FUN_00015dcc();
    local_4 = CONCAT22(uVar5 + extraout_A1_01[2],local_4._2_2_);
    uStack_8 = CONCAT22(*extraout_A1_01 + 0xe0,uStack_8._2_2_);
    uStack_14 = 0xe0;
    uStack_10 = uVar5;
    uStack_c = uVar3;
    sVar4 = FUN_0000e7c8();
    uVar5 = uStack_10;
    puVar9 = (ushort *)auStack_a;
    iVar2 = extraout_A0_07;
    unaff_D2w = uStack_14;
    sVar6 = sStack_e;
    uVar3 = uStack_c;
    if ((sVar4 != 0) && (-1 < sVar4)) {
      unaff_D2w = -uStack_14;
      sStack_e = extraout_A1_02[2] - sStack_12;
      sStack_12 = unaff_D2w + *extraout_A1_02;
      sVar4 = FUN_0000e7c8();
      sVar6 = 1;
      puVar9 = &uStack_14;
      iVar2 = extraout_A0_08;
      uVar3 = uVar5;
      if ((sVar4 != 0) && (-1 < sVar4)) {
        sVar6 = *extraout_A1_03;
        uVar3 = extraout_A1_03[2];
      }
    }
    *(short *)(iVar2 + 0x96) = sVar6;
    *(ushort *)(iVar2 + 0x98) = uVar3;
    bVar10 = false;
    puVar9[-0xffffffff00000002] = 1;
    puVar9[-0xffffffff00000001] = 0x459c;
    FUN_000144e8();
    puVar7 = extraout_A0_09;
    register0x0000003c = (BADSPACEBASE *)puVar9;
    if (bVar10) {
      return;
    }
  }
  else {
    if ((in_A0[0x4f] & 0x4000) != 0) {
      local_4 = 0x139ce;
      FUN_0001dbaa();
      in_A0 = extraout_A0;
    }
    if (_DAT_00ffdf84 != 0) goto LAB_000139d8;
    local_4 = 0x13ef8;
    FUN_00013f76();
    bVar10 = *(byte *)((int)extraout_A0_02 + 0x4d) < 2;
    in_A0 = extraout_A0_02;
    if ('\x02' < (char)*(byte *)((int)extraout_A0_02 + 0x4d)) {
      if (*(char *)((int)extraout_A0_02 + 0x4d) == '\x04') {
        unaff_D2w = *DAT_00fffc04;
        local_4 = 0x13f24;
        uVar3 = compute_diagonal_step_cost();
        bVar10 = uVar3 < 0x500;
        in_A0 = extraout_A0_03;
        if (bVar10) {
          local_4 = 0x13f32;
          FUN_0001dbaa();
          in_A0 = extraout_A0_04;
LAB_000139d8:
          if (*(int *)(in_A0 + 0x56) == 0) {
            local_4 = 0x139e4;
            iVar2 = FUN_00013ffe();
            in_A0 = extraout_A0_00;
          }
          else {
            local_4 = 0x13a0a;
            measure_relative_heading_and_distance();
            uStack_8 = 0x13a10;
            local_4 = extraout_A1;
            iVar2 = FUN_00013ffe();
            in_A0 = extraout_A0_01;
          }
          if (iVar2 != 0x12345678) {
            FUN_00013d0c();
            return;
          }
          puVar7 = DAT_00fffc04;
          if (*(ushort **)(in_A0 + 0x51) != (ushort *)0x0) {
            puVar7 = *(ushort **)(in_A0 + 0x51);
          }
          goto LAB_0001450e;
        }
      }
      else {
        if ('\x04' < *(char *)((int)extraout_A0_02 + 0x4d)) goto LAB_000139d8;
        uVar3 = *DAT_00fffc04;
        unaff_D2w = DAT_00fffc04[2];
        local_4 = 0x13f52;
        measure_relative_heading_and_distance();
        bVar10 = uVar3 < 0x640;
        in_A0 = extraout_A0_05;
        if (bVar10) {
          if ((short)extraout_A0_05[0xb] < 0x3d) {
            extraout_A0_05[0x1d] = 0;
            extraout_A0_05[0x1e] = 0xb45c;
            extraout_A0_05[0x17] = unaff_D2w;
            return;
          }
          return;
        }
      }
    }
LAB_000142ae:
    puVar7 = DAT_00fffc04;
    if ((in_A0[0x4f] & 0x4000) != 0) goto LAB_0001450e;
    local_4 = 0x142ca;
    FUN_000144e8();
    puVar7 = extraout_A0_06;
    if (bVar10) {
      in_A0 = extraout_A0_06;
      puVar7 = extraout_A1_00;
      if (((extraout_A0_06[8] & 0x2000) != 0) &&
         (psVar8 = *(short **)(extraout_A0_06 + 0x70), puVar7 = DAT_00fffc04, -1 < *psVar8)) {
        uVar3 = extraout_A0_06[0x72] + 4;
        if (*(short *)((int)psVar8 + (int)(short)uVar3) == 0) {
          uVar3 = 0;
        }
        extraout_A0_06[0x72] = uVar3;
        extraout_A0_06[0x4b] = *(ushort *)((int)psVar8 + (int)(short)uVar3);
        extraout_A0_06[0x4c] = *(ushort *)((int)psVar8 + (short)uVar3 + 2);
        return;
      }
      goto LAB_0001450e;
    }
  }
  uVar11 = puVar7[0xb] < 0x3c;
  if (0x3c < (short)puVar7[0xb]) {
    return;
  }
  if ((puVar7[0x58] & 0x200) == 0) {
    bVar1 = *(byte *)(puVar7 + 0x58);
    *(byte *)(puVar7 + 0x58) = bVar1 & 0xfb;
    if ((bVar1 & 4) == 0) {
      uVar11 = puVar7[0x5c] < 0x1000;
      if (puVar7[0x5c] != 0x1000) {
        *(undefined4 *)((int)register0x0000003c + -4) = 0x145da;
        update_entity_axis_relation_state();
        puVar7 = extraout_A0_11;
        if ((extraout_A0_11[0x58] & 0x200) != 0) goto FUN_000149c0;
        *(undefined4 *)((int)register0x0000003c + -4) = 0x145e8;
        derive_entity_axis_relation_flags();
        puVar7 = extraout_A0_12;
      }
    }
    else {
      *(undefined4 *)((int)register0x0000003c + -4) = 0x145c8;
      derive_entity_axis_relation_flags();
      puVar7 = extraout_A0_10;
    }
LAB_000145e8:
    uVar5 = puVar7[0x4b];
    uVar3 = puVar7[0x4c];
    *(undefined4 *)((int)register0x0000003c + -4) = 0x145fc;
    measure_relative_heading_and_distance();
    iVar2 = extraout_A0_13;
  }
  else {
FUN_000149c0:
    if ((puVar7[0x58] & 0x800) != 0) {
      *(undefined4 *)((int)register0x0000003c + -4) = 0x149ce;
      is_entity_path_blocked();
      puVar7 = extraout_A0_14;
      if ((bool)uVar11) {
        *(byte *)(extraout_A0_14 + 0x58) = *(byte *)(extraout_A0_14 + 0x58) & 0xfd;
        *(byte *)(extraout_A0_14 + 0x58) = *(byte *)(extraout_A0_14 + 0x58) & 0xf7;
        return;
      }
    }
    sVar6 = (puVar7[2] >> 4 & 0x7fe0) * 2 + (*puVar7 >> 9);
    psVar8 = (short *)((int)puVar7 + (short)puVar7[0x5d] + 0xbc);
    if (sVar6 == *psVar8) {
      if (psVar8[1] == -1) {
        if ((puVar7[0x58] & 0x800) != 0) {
          *(undefined4 *)((int)register0x0000003c + -4) = 0x14a64;
          compute_obstruction_grid_index();
          uVar3 = *(ushort *)(extraout_A0_17 + 0x34) >> 0xd & 6;
          if (((&DAT_00ff1700)[(short)(*(short *)(&DAT_000148ca + (short)uVar3) + sVar6)] == '\0')
             && ((&DAT_00ff1700)
                 [(short)(*(short *)(&DAT_000148ca + (short)(uVar3 + 2 & 6)) +
                         *(short *)(&DAT_000148ca + (short)(uVar3 + 4 & 6)) + sVar6)] == '\0')) {
            *(undefined4 *)((int)register0x0000003c + -4) = 0x14ab0;
            find_first_occupied_obstruction_tile();
            sVar4 = extraout_D1w;
            iVar2 = extraout_A0_18;
          }
          else {
            sVar4 = 4;
            while( true ) {
              *(short *)((int)register0x0000003c + -2) = sVar4;
              *(undefined4 *)((int)register0x0000003c + -6) = 0x14ac6;
              find_alternate_route_tile();
              sVar4 = extraout_D1w_00;
              iVar2 = extraout_A0_19;
              if (extraout_D1w_00 != sVar6) break;
              sVar4 = *(short *)((int)register0x0000003c + -2) + -1;
              if (sVar4 == -1) {
                clear_entity_route_flags();
                return;
              }
            }
          }
          *(undefined2 *)(iVar2 + 0xba) = 0;
          *(short *)(iVar2 + 0xbc) = sVar4;
          *(undefined2 *)(iVar2 + 0xbe) = 0xffff;
          return;
        }
        *(byte *)(puVar7 + 0x58) = *(byte *)(puVar7 + 0x58) & 0xfd;
        *(byte *)(puVar7 + 0x58) = *(byte *)(puVar7 + 0x58) & 0xf7;
        *(undefined4 *)((int)register0x0000003c + -4) = 0x14a50;
        derive_entity_axis_relation_flags();
        puVar7 = extraout_A0_16;
        goto LAB_000145e8;
      }
      puVar7[0x5d] = puVar7[0x5d] + 2;
    }
    *(undefined4 *)((int)register0x0000003c + -4) = 0x14a12;
    decode_obstruction_tile_projection();
    *(ushort *)((int)register0x0000003c + -2) = unaff_D2w << 0xd | unaff_D2w >> 3;
    *(undefined4 *)((int)register0x0000003c + -6) = 0x14a2e;
    uVar5 = compute_diagonal_step_cost();
    uVar3 = *(ushort *)((int)register0x0000003c + -2);
    iVar2 = extraout_A0_15;
  }
  *(ushort *)(iVar2 + 0x34) = uVar3;
  *(ushort *)(iVar2 + 200) = uVar5;
  *(undefined1 **)(iVar2 + 0x3a) = &LAB_0000b574;
  return;
}


