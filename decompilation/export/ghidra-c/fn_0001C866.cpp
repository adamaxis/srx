// Ghidra headless export
// Entry: 0x0001C866
// Name: advance_title_effect_motion
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00019E32 run_menu_selection_service
//
// ROM trace: FFE066 is a four-tick countdown for FFE064's twelve frames.
// Before rendering, 0x1C8D8..0x1C904 selects the direct stream table at
// 0x1C926 or 0x1C956 from the nonzero FFE0E2 mode and the FFE060 < 0x120
// position boundary, then indexes the selected table by frame. Completion
// writes 0x1C288 to the secondary behavior slot (+8/+0x0C) on the left-side
// path, or 0x1C28E to the primary slot (+4/+0x0D) on the right-side path,
// clears FFE0D6, calls the menu service, and clears FFE05E.


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

