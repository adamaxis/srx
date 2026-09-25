// Ghidra headless export
// Entry: 0x0000B48C
// Name: evaluate_entity_collision
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000E7C8 evaluate_terrain_collision


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void evaluate_entity_collision(void)

{
  int extraout_A0;
  
  evaluate_terrain_collision();
  *(undefined4 *)(extraout_A0 + 0xd0) = _DAT_00ffef10;
  return;
}


