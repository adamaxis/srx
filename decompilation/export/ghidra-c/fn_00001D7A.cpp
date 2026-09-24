// Ghidra headless export
// Entry: 0x00001D7A
// Name: setup_title_state
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000012F6 advance_title_palette_fade
//   0x00001406 prepare_title_resources
//   0x00001CFC load_title_resources
//   0x0000C0C2 load_title_font


void setup_title_state(void)

{
  DAT_00ffdd08 = 0;
  DAT_00ffdd0c = 0;
  DAT_00ffdd0a = 0;
  DAT_00ffdd0e = 0;
  prepare_title_resources();
  load_title_resources();
  load_title_font();
  DAT_00ffef14 = DAT_00ffef14 | 0xd;
  advance_title_palette_fade();
  return;
}


