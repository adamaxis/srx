// Ghidra headless export
// Entry: 0x00014F50
// Name: FUN_00014f50
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014F26 probe_entity_world_position
//   0x00015ABA find_colliding_entity


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00014f50(void)

{
  char cVar1;
  short sVar2;
  undefined4 unaff_D5;
  undefined4 unaff_D6;
  undefined4 *extraout_A0;
  
  sVar2 = probe_entity_world_position();
  if (((sVar2 == 0) || (_DAT_00ffdfe2 != 0)) &&
     (cVar1 = *(char *)((int)extraout_A0 + 0x89) + -1, *(char *)((int)extraout_A0 + 0x89) = cVar1,
     -1 < cVar1)) {
    *extraout_A0 = unaff_D5;
    extraout_A0[1] = unaff_D6;
    find_colliding_entity();
    return;
  }
  return;
}


