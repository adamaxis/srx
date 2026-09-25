// Ghidra headless export
// Entry: 0x00004ADC
// Name: advance_entity_effect_timer
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x00005A56 project_entity_screen_position
//   0x000061A0 append_entity_sprite_record


void advance_entity_effect_timer(void)

{
  short sVar1;
  ushort uVar2;
  undefined2 extraout_D1u;
  undefined4 uVar3;
  uint uVar4;
  uint uVar5;
  undefined4 uVar6;
  ushort uVar7;
  short sVar8;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int iVar9;
  int extraout_A0_01;
  undefined1 *puVar10;
  int iVar11;
  short *psVar12;
  
  sVar1 = *(short *)(in_A0 + 0xe) + -1;
  *(short *)(in_A0 + 0xe) = sVar1;
  if (sVar1 < 0) {
    *(undefined2 *)(in_A0 + 2) = 0;
    clear_effect_slot();
    sVar1 = *(short *)(extraout_A0_01 + 0x34);
    sVar8 = 4;
    puVar10 = (undefined1 *)(extraout_A0_01 + 0x36);
    psVar12 = *(short **)
               ((int)&PTR_DAT_00004bb0 + (int)(short)(*(short *)(extraout_A0_01 + 6) << 2));
    do {
      (&DAT_00ff1700)[(short)(*psVar12 + sVar1)] = *puVar10;
      sVar8 = sVar8 + -1;
      puVar10 = puVar10 + 1;
      psVar12 = psVar12 + 1;
    } while (sVar8 != -1);
    return;
  }
  project_entity_screen_position();
  iVar11 = *(int *)((int)&PTR_DAT_00004acc + (int)(short)((*(ushort *)(extraout_A0 + 0xe) & 3) << 2)
                   ) + 8;
  uVar3 = CONCAT22(extraout_D1u,*(undefined2 *)(extraout_A0 + 10));
  uVar4 = (uint)*(ushort *)(extraout_A0 + 0xc);
  uVar6 = 0x2000;
  uVar5 = (uint)*(ushort *)(extraout_A0 + 0x2a);
  uVar7 = 4;
  iVar9 = extraout_A0;
  do {
    uVar2 = *(ushort *)(iVar9 + 0xe) % 5;
    if (uVar7 == uVar2) {
LAB_00004b50:
      build_title_sprite_records_with_attribute_offset(uVar3,uVar4,uVar5,uVar6,uVar7,iVar11);
      iVar9 = extraout_A0_00;
    }
    else {
      uVar2 = uVar2 + 1;
      if (uVar2 == 5) {
        uVar2 = 0;
      }
      if (uVar7 == uVar2) goto LAB_00004b50;
    }
    uVar7 = uVar7 - 1;
    if (uVar7 == 0xffff) {
      append_entity_sprite_record();
      return;
    }
  } while( true );
}


