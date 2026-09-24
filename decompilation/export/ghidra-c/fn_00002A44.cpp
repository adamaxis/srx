// Ghidra headless export
// Entry: 0x00002A44
// Name: initialize_field_mode_profile_b
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000136DE initialize_scene_mode_state
//   0x00053126 prepare_mode_entry_service


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void initialize_field_mode_profile_b(void)

{
  DAT_00ffe114 = &UNK_00002b24;
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


