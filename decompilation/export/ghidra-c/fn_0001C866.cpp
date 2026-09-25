// Ghidra headless export
// Entry: 0x0001C866
// Name: advance_title_effect_motion
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00019E32 run_menu_selection_service


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_title_effect_motion(void)

{
  DAT_00ffe066 = DAT_00ffe066 + -1;
  if (DAT_00ffe066 < 0) {
    DAT_00ffe066 = 3;
    DAT_00ffe064 = DAT_00ffe064 + 1;
    if (0xb < DAT_00ffe064) {
      if (DAT_00ffe060 < 0x120) {
        DAT_00ff1588 = &DAT_0001c288;
        DAT_00ff158c = 0;
      }
      else {
        DAT_00ff1584 = &DAT_0001c28e;
        DAT_00ff158d = 0;
      }
      _DAT_00ffe0d6 = 0;
      run_menu_selection_service();
      _DAT_00ffe05e = 0;
      return;
    }
  }
  build_title_sprite_records();
  return;
}

