// Ghidra headless export
// Entry: 0x0001651E
// Name: convert_world_routine_0x0001651e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000164F2 entity_table_has_free_slot
//   0x0001669C project_entity_x_axis
//   0x000166D0 project_entity_y_axis


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_world_routine_0x0001651e(void)

{
  short in_D0w;
  short sVar1;
  short unaff_D4w;
  int in_A0;
  undefined2 *extraout_A0;
  undefined2 *puVar2;
  int unaff_A2;
  
  if (DAT_00ffdf40 == 0) {
    if ((DAT_00ffddc4 == 4) && ((*(ushort *)(in_A0 + 0xc6) & 0x1f) == 6)) {
      DAT_00ffe114 = &UNK_0001667a;
    }
    if ((*(ushort *)(in_A0 + 0xc6) & 0xe0) == 0xc0) {
      entity_table_has_free_slot();
      *extraout_A0 = 0xffff;
      extraout_A0[3] = DAT_00ffdd16 + unaff_D4w;
      extraout_A0[6] = 3;
      extraout_A0[7] = 3;
      *(undefined1 **)(extraout_A0 + 4) = &LAB_0001669a;
    }
  }
  else if (in_D0w != 0) {
    if (DAT_00ffddc4 == 0x14) {
      *(undefined1 *)(unaff_A2 + DAT_00ffdfe4) = 0;
      sVar1 = project_entity_y_axis();
      *(undefined2 *)((int)&DAT_00ff2700 + (int)sVar1) = DAT_001cd296;
      *(undefined2 *)((int)DAT_00ffddea + (int)&DAT_00ff2700 + (int)sVar1) = DAT_001cd298;
      _DAT_00ffe100 = 1;
    }
    else if (DAT_00ffdf40 == 2) {
      *(undefined1 *)(unaff_A2 + DAT_00ffdfe4) = 0;
      if ((*(ushort *)(in_A0 + 0xc6) & 0xe0) == 0xa0) {
        sVar1 = project_entity_x_axis();
        puVar2 = (undefined2 *)((int)&DAT_00ff4700 + ((int)sVar1 - (int)DAT_00ffddea));
        *puVar2 = 3;
        sVar1 = DAT_00ffddea;
        puVar2 = (undefined2 *)((int)DAT_00ffddea + (int)puVar2);
        *puVar2 = 4;
        *(undefined2 *)((int)sVar1 + (int)puVar2) = 5;
        _DAT_00ffe102 = 1;
      }
      else {
        sVar1 = project_entity_y_axis();
        *(undefined2 *)((int)&DAT_00ff2700 + (int)sVar1) = 6;
        *(undefined2 *)((int)DAT_00ffddea + (int)&DAT_00ff2700 + (int)sVar1) = 7;
        _DAT_00ffe100 = 1;
      }
    }
    return;
  }
  return;
}


