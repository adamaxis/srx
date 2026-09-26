// Ghidra headless export
// Entry: 0x00015646
// Name: spawn_entity_effect_record
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000036FC advance_entity_asset_meter
//   0x00015118 initialize_entity_behavior_record
//   0x00015A8C find_first_available_entity_slot
//   0x00015D3C measure_relative_heading_and_distance
//   0x0001A2CC allocate_first_free_effect_slot
//   0x0001DBAA schedule_world_activity_timeout
//   0x00055848 calculate_effect_target_offset


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void spawn_entity_effect_record(void)

{
  short *psVar1;
  short sVar2;
  ushort uVar3;
  short sVar4;
  undefined2 uVar5;
  ushort uVar6;
  short sVar7;
  short unaff_D5w;
  int in_A0;
  undefined2 *extraout_A0;
  int extraout_A0_00;
  undefined2 *extraout_A1;
  short *extraout_A1_00;
  int extraout_A1_01;
  short unaff_A2w;
  undefined1 uVar8;
  bool bVar9;
  undefined6 uVar10;
  
  if ((*(ushort *)(in_A0 + 0x16) & 0x7ff) < 0x3d) {
    uVar8 = 0;
    advance_entity_asset_meter();
    find_first_available_entity_slot();
    if (!(bool)uVar8) {
      schedule_world_activity_timeout();
      uVar5 = initialize_entity_behavior_record();
      *(undefined1 **)(extraout_A1 + 0x1d) = &LAB_00015390;
      extraout_A1[0x16] = 0x2000;
      *extraout_A1 = *extraout_A0;
      extraout_A1[2] = extraout_A0[2];
      extraout_A1[0x17] = extraout_A0[0x17];
      *(undefined1 *)((int)extraout_A1 + 0x9d) = *(undefined1 *)((int)extraout_A0 + 0x9d);
      *(undefined1 *)((int)extraout_A1 + 0x57) = *(undefined1 *)((int)extraout_A0 + 0x57);
      *(undefined1 *)(extraout_A1 + 0x2b) = *(undefined1 *)(extraout_A0 + 0x2b);
      extraout_A1[0x1f] = uVar5;
      extraout_A1[0x23] = 0x13;
      extraout_A1[0x1b] = 0xfb00;
      extraout_A1[0x1a] = 0xf400;
      extraout_A1[7] = 2;
      extraout_A1[0x20] = 0xb4;
      extraout_A1[0xb] = 0;
      bVar9 = false;
      uVar10 = allocate_first_free_effect_slot();
      if (!bVar9) {
        extraout_A1_00[0x18] = (short)((uint6)uVar10 >> 0x20);
        extraout_A1_00[0x10] = (short)uVar10;
        extraout_A1_00[0x13] = unaff_A2w;
        uVar3 = extraout_A1_00[0x17];
        uVar6 = uVar3 << 3 | uVar3 >> 0xd;
        if ((short)uVar6 < 0) {
          uVar6 = uVar6 + 1;
        }
        sVar7 = (uVar6 & 7) * 4;
        DAT_00ffdd60 = *extraout_A1_00 + *(short *)(&DAT_00015a6c + sVar7);
        DAT_00ffdd62 = extraout_A1_00[2] + *(short *)(&DAT_00015a6c + (short)(sVar7 + 2));
        *extraout_A1_00 = *(short *)(&DAT_00015a4c + sVar7) + *extraout_A1_00;
        sVar7 = *(short *)(&DAT_00015a4c + (short)(sVar7 + 2));
        extraout_A1_00[2] = sVar7 + extraout_A1_00[2];
        psVar1 = *(short **)(extraout_A0_00 + 0xa2);
        if (psVar1 == (short *)0x0) {
          *(byte *)((int)extraout_A1_00 + 0x9f) = *(byte *)(extraout_A0_00 + 0x87) >> 1;
          uVar3 = uVar3 >> 8 & 0xfffe;
          sVar7 = *(short *)((int)&DAT_00015e16 + (int)(short)uVar3);
          extraout_A1_00[0x26] =
               (short)(*(short *)((int)&DAT_00015e56 + (int)(short)uVar3) * 8 >> 5);
          extraout_A1_00[0x27] = (short)(sVar7 * 8 >> 5);
          return;
        }
        sVar2 = *psVar1;
        sVar4 = psVar1[2];
        calculate_effect_target_offset(psVar1,uVar3);
        *(undefined1 *)((int)extraout_A1_00 + 0x9f) = DAT_00fff0dd;
        uVar3 = unaff_D5w * 0x10 + sVar4;
        uVar6 = sVar7 * 0x10 + sVar2;
        measure_relative_heading_and_distance();
        if (0xbdf < uVar6) {
          uVar6 = 0xbe0;
        }
        *(ushort *)(extraout_A1_01 + 0x2e) = uVar3;
        *(ushort *)(extraout_A1_01 + 0x40) = (uVar6 / 0x80) * 2;
        uVar3 = uVar3 >> 8 & 0xfffe;
        sVar7 = *(short *)((int)&DAT_00015e16 + (int)(short)uVar3);
        *(short *)(extraout_A1_01 + 0x4c) =
             (short)(*(short *)((int)&DAT_00015e56 + (int)(short)uVar3) * 8 >> 4);
        *(short *)(extraout_A1_01 + 0x4e) = (short)(sVar7 * 8 >> 4);
        *(ushort *)(extraout_A1_01 + 0x36) = (uVar6 / 0x80) * -0x20;
        return;
      }
      *(undefined1 *)(extraout_A1_00 + 8) = 0;
    }
  }
  return;
}


