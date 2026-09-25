// Ghidra headless export
// Entry: 0x0001BEBA
// Name: convert_world_routine_0x0001beba
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00019D66 run_menu_slot_service
//   0x0001A908 convert_world_routine_0x0001a908
//   0x0001B612 attach_behavior_descriptor_b
//   0x0001BF5C copy_title_profile


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_world_routine_0x0001beba(void)

{
  short sVar1;
  undefined2 *puVar2;
  undefined2 *puVar3;
  
  sVar1 = 6;
  puVar2 = (undefined2 *)((int)&DAT_0001c10a + (int)(short)((DAT_00ffdf88 & 7) << 1));
  puVar3 = (undefined2 *)((int)&DAT_00ffd87e + 2);
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  DAT_00ffe00c = DAT_00ffe00c + -1;
  if (DAT_00ffe00c < 0) {
    copy_title_profile();
    DAT_00ff1592 = DAT_00ff1592 & 0x7f;
    attach_behavior_descriptor_b();
    _DAT_00ffe0d6 = 0;
    run_menu_slot_service();
    convert_world_routine_0x0001a908();
    return;
  }
  build_title_sprite_records
            (*(undefined4 *)(&DAT_0001c136 + (short)(((ushort)-DAT_00ffe00c >> 1 & 3) << 2)));
  build_title_sprite_records();
  return;
}


