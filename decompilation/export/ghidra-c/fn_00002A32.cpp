// Ghidra headless export
// Entry: 0x00002A32
// Name: initialize_field_mode_profile_a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void initialize_field_mode_profile_a(void)

{
  DAT_00ffe114 = &UNK_00002aea;
  prepare_mode_entry_service();
  DAT_00ffe12e = 0;
  DAT_00ffe180 = 0;
  DAT_00ffe182 = 0;
  DAT_00ffe184 = 0;
  DAT_00ffe187 = 0;
  DAT_00ffe18f = 0;
  DAT_00ffe183 = 0;
  _DAT_00ffdf84 = 0;
  initialize_scene_mode_state();
  return;
}


