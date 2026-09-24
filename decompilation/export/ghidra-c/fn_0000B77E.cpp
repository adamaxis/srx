// Ghidra headless export
// Entry: 0x0000B77E
// Name: probe_entity_directional_collision
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000B48C FUN_0000b48c


short probe_entity_directional_collision(void)

{
  undefined2 in_D0w;
  short sVar1;
  int extraout_A0;
  
  DAT_00ffdf66 = in_D0w;
  sVar1 = FUN_0000b48c();
  if ((sVar1 < 0) && ((*(byte *)(extraout_A0 + 0x10) & 2) == 0)) {
    sVar1 = 0x7f;
  }
  return sVar1;
}


