// Ghidra headless export
// Entry: 0x0005A1EA
// Name: FUN_0005a1ea
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005A252 advance_title_intro_persistent_sequence
//   0x0005A3BE advance_title_intro_motion
//   0x0005A4C2 advance_title_intro_dual_sprites
//   0x0005A59C advance_title_intro_timed_sprites
//   0x0005A628 advance_title_intro_targeted_sweep
//   0x0005A71C advance_title_intro_linear_sweep_a
//   0x0005A7F8 advance_title_intro_linear_sweep_b
//   0x0005A8DE advance_title_intro_final_projection


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005a1ea(void)

{
  DAT_00ffdd02 = 1;
  advance_title_intro_persistent_sequence();
  advance_title_intro_motion();
  advance_title_intro_dual_sprites();
  advance_title_intro_timed_sprites();
  advance_title_intro_targeted_sweep();
  advance_title_intro_linear_sweep_a();
  advance_title_intro_linear_sweep_b();
  advance_title_intro_final_projection();
  _DAT_00ffd980 = 0;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  return;
}


