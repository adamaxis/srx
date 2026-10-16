// Ghidra headless export
// Entry: 0x0005B0BC
// Name: service_title_intro_animation
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005B100 advance_title_intro_pulse
//   0x0005B1B6 advance_title_intro_landing


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void service_title_intro_animation(void)

{
  DAT_00ffdd02 = 1;
  advance_title_intro_pulse();
  advance_title_intro_landing();
  _DAT_00ffd980 = 0;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  return;
}


