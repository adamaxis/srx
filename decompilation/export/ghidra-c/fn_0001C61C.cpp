// Ghidra headless export
// Entry: 0x0001C61C
// Name: advance_title_sweep_animation
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00019CB2 run_menu_selection_code_four
//   0x0001B612 attach_behavior_descriptor_b
//
// ROM trace: records at 0x1C6D4 are four bytes each: a source index into the
// six direct streams at 0x1C6BC, a signed horizontal offset, and a word whose
// low byte becomes FF1580+0x14. The phase advances before the record is read;
// phase 0x20 clears record bits 0 and 3, attaches behavior B, clears FFE0D6,
// and calls menu-selection code four. Otherwise bit 3 suppresses both
// 0x1378 calls; an active phase builds the same stream at base X plus and
// minus the signed offset.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_title_sweep_animation(void)

{
  DAT_00ff1594 = DAT_00ff1594 + -1;
  if (DAT_00ff1594 < '\0') {
    DAT_00ff1593 = DAT_00ff1593 + '\x01';
    DAT_00ff1594 = (char)*(undefined2 *)
                          ((int)&PTR_DAT_0001c6d6 + (int)(short)((short)DAT_00ff1593 << 2));
    if ('\x1f' < DAT_00ff1593) {
      DAT_00ff1592 = DAT_00ff1592 & 0xf6;
      attach_behavior_descriptor_b();
      _DAT_00ffe0d6 = 0;
      run_menu_selection_code_four();
      return;
    }
  }
  if ((DAT_00ff1592 & 8) != 0) {
    return;
  }
  build_title_sprite_records();
  build_title_sprite_records();
  return;
}

