// Ghidra headless export
// Entry: 0x0001CA12
// Name: advance_world_effect_motion
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00019E32 run_menu_selection_service
//
// ROM trace: the live countdown FFE070 finalizes through 0x19E32 when its
// decremented signed word is negative. Otherwise FFE072 selects a byte from
// 0x1CA8C; a nonnegative FFE06C adds five to that index before the direct
// stream pointer is loaded from 0x1CAAC. The source then adds FFE06C to the
// fixed-point X, subtracts 0x1A from the fixed-point Y, shifts both by four,
// and builds with 0x2000.


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

