// Ghidra headless export
// Entry: 0x00013F76
// Name: convert_menu_routine_0x00013f76
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014D8E convert_entity_routine_0x00014d8e
//   0x0001DBAA schedule_world_activity_timeout
//   0x0001E2E0 begin_world_activity_if_idle


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_menu_routine_0x00013f76(void)

{
  ushort uVar1;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int iVar2;
  bool bVar3;
  
  if (_DAT_00ffdf84 == 0) {
    bVar3 = false;
    if (DAT_00ffe16f != '\0') {
      schedule_world_activity_timeout();
      return;
    }
    if ((((*(byte *)(in_A0 + 0x9e) & 1) == 0) && ((*(byte *)(in_A0 + 0x10) & 0x20) != 0)) &&
       (convert_entity_routine_0x00014d8e(), !bVar3)) {
      uVar1 = *(short *)(extraout_A0 + 0x2e) - DAT_00ff13ae;
      if ((short)uVar1 < 0) {
        uVar1 = -uVar1;
      }
      if (uVar1 < 0x4000) {
        iVar2 = extraout_A0;
        if (DAT_00ffdf40 == 2) {
          if ((*(byte *)(DAT_00fffc04 + 0x11) & 0x80) != 0) {
            return;
          }
          _DAT_00ffdd2c = &LAB_00020fec;
          begin_world_activity_if_idle();
          iVar2 = extraout_A0_00;
        }
        *(byte *)(iVar2 + 0x9e) = *(byte *)(iVar2 + 0x9e) | 1;
        return;
      }
    }
  }
  return;
}


