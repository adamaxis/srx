// Ghidra headless export
// Entry: 0x0001DD94
// Name: FUN_0001dd94
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00016ECA process_terrain_work_batch
//   0x0001DBF0 reset_entity_service_records
//   0x0001DC4C FUN_0001dc4c
//   0x0001DEA0 clear_inactive_entity_effects
//   0x0001DF1A resolve_nearby_effects
//   0x0001DFC4 advance_world_effect_trigger
//   0x0001E166 FUN_0001e166
//   0x0001E2EC begin_world_activity
//   0x00053126 prepare_mode_entry_service


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001dd94(void)

{
  int iVar1;
  
  if (((DAT_00ffe186 == '\x02') && (DAT_00fff13e._2_2_ < 0)) && (DAT_00ff044c < '\n')) {
    DAT_00fff13e._2_2_ = 0;
    DAT_00ffe186 = '\x03';
  }
  if ((DAT_00ffe166 != 0) && (DAT_00ffe166 = DAT_00ffe166 + -1, DAT_00ffe166 == 0)) {
    prepare_mode_entry_service();
    _DAT_00ffdd2c = FUN_0000c64c;
    begin_world_activity();
  }
  if (DAT_00ffe11c != 0) {
    DAT_00ffe11c = DAT_00ffe11c + -1;
  }
  if (_DAT_00ffdf84 != 0) {
    if (((DAT_00ffe186 == '\0') && (DAT_00ffe16f == '\0')) &&
       ((DAT_00ffddc4 != 0x14 && (DAT_00ffdf86 = DAT_00ffdf86 + -1, DAT_00ffdf86 < 0)))) {
      reset_entity_service_records();
    }
    else {
      iVar1 = *(int *)(DAT_00fffc04 + 0xa2);
      if (((iVar1 == 0) || ((*(byte *)(iVar1 + 0x10) & 1) == 0)) ||
         (((-1 < *(char *)(DAT_00fffc04 + 0x56) || (*(char *)(DAT_00fffc04 + 0x57) != '\f')) &&
          ((*(byte *)(iVar1 + 0x10) & 0x40) != 0)))) {
        FUN_0001dc4c();
      }
    }
  }
  clear_inactive_entity_effects();
  if (DAT_00ffdf40 == 2) {
    process_terrain_work_batch();
    advance_world_effect_trigger();
  }
  FUN_0001e166();
  resolve_nearby_effects();
  return;
}


