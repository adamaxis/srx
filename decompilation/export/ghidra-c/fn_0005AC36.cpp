// Ghidra headless export
// Entry: 0x0005AC36
// Name: advance_title_intro_stage_projection
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


void advance_title_intro_stage_projection(void)

{
  build_title_sprite_records();
  DAT_00fff186 = DAT_00fff186 + 0x40;
  DAT_00fff188 = DAT_00fff188 + 0x18;
  return;
}


