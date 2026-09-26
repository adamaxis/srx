// Ghidra headless export
// Entry: 0x00014D8E
// Name: probe_entity_spawn_collision_window
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014F26 probe_entity_world_position
//   0x00014FF8 initialize_entity_spawn_record
//   0x00015ABA find_colliding_entity


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void probe_entity_spawn_collision_window(void)

{
  short sVar1;
  uint unaff_D5;
  uint unaff_D6;
  uint *extraout_A0;
  undefined4 in_A1;
  bool bVar2;
  short *psVar3;
  undefined4 *puVar4;
  short local_8 [2];
  undefined4 local_4;
  
  _DAT_00ffdfd8 = 0xffff;
  local_4 = 0x14dac;
  _DAT_00ffdd3c = in_A1;
  initialize_entity_spawn_record();
  puVar4 = &local_4;
  sVar1 = 0x1a;
  do {
    *(short *)((int)puVar4 + -2) = sVar1;
    *(undefined4 *)((int)puVar4 + -6) = 0x14dbe;
    sVar1 = probe_entity_world_position();
    *extraout_A0 = unaff_D5;
    extraout_A0[1] = unaff_D6;
    unaff_D5 = unaff_D5 << 0x10 | unaff_D5 >> 0x10;
    unaff_D6 = unaff_D6 << 0x10 | unaff_D6 >> 0x10;
    if ((sVar1 != 0) && (_DAT_00ffdfe2 == 0)) {
      _DAT_00ffdfd8 = 0;
      return;
    }
    bVar2 = false;
    psVar3 = (short *)((int)puVar4 + -4);
    *psVar3 = *(short *)((int)puVar4 + -2);
    *(undefined4 *)((int)puVar4 + -8) = 0x14de2;
    find_colliding_entity();
    puVar4 = (undefined4 *)((int)puVar4 + -2);
  } while ((!bVar2) && (sVar1 = *psVar3 + -1, sVar1 != -1));
  _DAT_00ffdfd8 = 0;
  return;
}

