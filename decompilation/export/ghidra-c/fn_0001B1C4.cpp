// Ghidra headless export
// Entry: 0x0001B1C4
// Name: initialize_matrix_program_transition
// Previous generated label: initialize_world_effect_seed
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001A908 initialize_world_effect_mode


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void initialize_matrix_program_transition(void)

{
  undefined2 in_D1w;
  short unaff_D2w;
  
  // D2 is the selected Matrix-program slot.  The five-word table at 0x1B278
  // supplies the transition resource position; D1 is the requested mode.
  // These are the typed FFE0D8..FFE0E4 transition fields, not an effect
  // particle seed.
  _DAT_00ffe0de = 1;
  DAT_00ffe0dc = 0;
  DAT_00ffe0da = 0xc;
  DAT_00ffe0d8 = *(undefined2 *)(&DAT_0001b278 + (short)(unaff_D2w << 1));
  DAT_00ffe0e0 = in_D1w;
  DAT_00ffe0e4 = unaff_D2w;
  initialize_world_effect_mode();
  return;
}

