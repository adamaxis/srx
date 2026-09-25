// Ghidra headless export
// Entry: 0x0001CA12
// Name: advance_world_effect_motion
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00019E32 run_menu_selection_service


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_world_effect_motion(void)

{
  DAT_00ffe070 = DAT_00ffe070 + -1;
  if (DAT_00ffe070 < 0) {
    _DAT_00ffe06e = 0;
    _DAT_00ffe0d6 = 0;
    run_menu_selection_service();
    return;
  }
  DAT_00ffe072 = DAT_00ffe072 + 1;
  DAT_00ffe06a = DAT_00ffe06c + DAT_00ffe06a;
  DAT_00ffe068 = DAT_00ffe068 + -0x1a;
  build_title_sprite_records();
  return;
}

