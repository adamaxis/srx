// Ghidra headless export
// Entry: 0x0005D938
// Name: advance_pre_title_animation
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378
//   0x0005D996 advance_pre_title_logo_phase


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_pre_title_animation(void)

{
  DAT_00ffdd02 = 1;
  FUN_00001378();
  FUN_00001378();
  advance_pre_title_logo_phase();
  _DAT_00ffd980 = 1;
  return;
}


