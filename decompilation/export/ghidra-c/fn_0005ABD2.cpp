// Ghidra headless export
// Entry: 0x0005ABD2
// Name: FUN_0005abd2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005AC0E draw_title_intro_stage_base_sprite
//   0x0005AC36 advance_title_intro_stage_projection
//   0x0005AC7E advance_title_intro_stage_timed_sprites


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005abd2(void)

{
  DAT_00ffdd02 = 1;
  advance_title_intro_stage_timed_sprites();
  draw_title_intro_stage_base_sprite();
  advance_title_intro_stage_projection();
  _DAT_00ffd980 = 0;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  return;
}


