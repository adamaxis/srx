// Ghidra headless export
// Entry: 0x0001CCDC
// Name: update_persistent_title_effect
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00019E32 run_menu_selection_service
//   0x0001D048 advance_world_effect_record
//   0x0001D0DE resolve_world_effect_sprite


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void update_persistent_title_effect(void)

{
  int extraout_A0;
  
  if (DAT_00ff15a4 == '\0') {
    return;
  }
  advance_world_effect_record();
  resolve_world_effect_sprite();
  build_title_sprite_records();
  if (*(char *)(extraout_A0 + 6) == '\0') {
    _DAT_00ffe0d6 = 0;
    run_menu_selection_service();
  }
  return;
}

