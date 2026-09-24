// Ghidra headless export
// Entry: 0x0000B732
// Name: apply_movement_result_policy
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000B77E probe_entity_directional_collision


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

ushort apply_movement_result_policy(void)

{
  ushort uVar1;
  ushort uVar2;
  
  uVar2 = probe_entity_directional_collision();
  if (((short)uVar2 < 0) &&
     (((_DAT_00ffdf84 != 0 || (uVar1 = uVar2 & 0x1f, 0x17 < uVar1)) ||
      (((DAT_00ffddc4 == 0x14 || (DAT_00ffdf40 == 2)) && ((1 < uVar1 && (uVar1 < 6)))))))) {
    return 0x7f;
  }
  return uVar2;
}


