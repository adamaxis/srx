// Ghidra headless export
// Entry: 0x0001B1C4
// Name: initialize_world_effect_seed
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001A908 convert_world_routine_0x0001a908


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void initialize_world_effect_seed(void)

{
  undefined2 in_D1w;
  short unaff_D2w;
  
  _DAT_00ffe0de = 1;
  DAT_00ffe0dc = 0;
  DAT_00ffe0da = 0xc;
  DAT_00ffe0d8 = *(undefined2 *)(&DAT_0001b278 + (short)(unaff_D2w << 1));
  DAT_00ffe0e0 = in_D1w;
  DAT_00ffe0e4 = unaff_D2w;
  convert_world_routine_0x0001a908();
  return;
}


