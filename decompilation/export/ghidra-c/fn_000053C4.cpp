// Ghidra headless export
// Entry: 0x000053C4
// Name: convert_graphics_routine_0x000053c4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00005468 clone_entity_spawn_variant_a
//   0x00005494 convert_graphics_routine_0x00005494
//   0x00005622 clear_entity_runtime_fields
//   0x0000AFF2 prepare_entity_auxiliary_behavior
//   0x0001A304 clear_effect_slot


void convert_graphics_routine_0x000053c4(void)

{
  short sVar1;
  int extraout_A0;
  undefined4 extraout_A0_00;
  int iVar2;
  
  if (-1 < DAT_00ffdff6) {
    DAT_00ffdff6 = DAT_00ffdff6 + -1;
    if (-1 < DAT_00ffdff6) {
      return;
    }
    DAT_00ffdff8 = DAT_00ffdff8 + -1;
  }
  convert_graphics_routine_0x00005494();
  if (*(char *)(extraout_A0 + 0xb2) != '\0') {
    return;
  }
  sVar1 = 8;
  iVar2 = extraout_A0;
  do {
    if (8 < *(short *)(iVar2 + 0x13e)) break;
    if (((*(char *)(iVar2 + 0x1de) == '\r') && ((*(byte *)(iVar2 + 0x110) & 1) != 0)) &&
       ((*(byte *)(iVar2 + 0x110) & 0x40) == 0)) {
      clone_entity_spawn_variant_a();
      prepare_entity_auxiliary_behavior(extraout_A0_00);
      return;
    }
    sVar1 = sVar1 + -1;
    iVar2 = iVar2 + 0x100;
  } while (sVar1 != -1);
  if (-1 < DAT_00ffdff8) {
    DAT_00ffdff6 = 0x1e;
    clone_entity_spawn_variant_a();
    return;
  }
  clear_effect_slot();
  clear_entity_runtime_fields();
  return;
}


