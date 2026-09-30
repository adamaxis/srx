// Ghidra headless export
// Entry: 0x0001BCE8
// Name: advance_title_profile_motion
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x0001A106 refresh_menu_selection_or_prepare
// Profile phase table at 0x001BE4E contains thirteen records:
//   pointer index 0, x offsets 0/4/8/C/10/14/14/14/14/14/14/14/14,
//   y offset -0x30. The pointer table at 0x001BE3E resolves index 0 to
//   0x001D74A; the moving phase table resolves phases 0..3 to
//   0x001D74A/0x001D754/0x001D75E/0x001D768.
// Movement easing indexes ROM 0x0015E56 at ((elapsed * 3) & 0xFE),
// multiplies the signed sample by 0x58, shifts left six, and subtracts
// the resulting high word from 0x1340.


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void advance_title_profile_motion(void)

{
  if (DAT_00ffe058 == 1) {
    DAT_00ff1594 = DAT_00ff1594 - 1;
    if (-1 < (short)((ushort)DAT_00ff1594 << 8)) {
LAB_0001bd50:
      build_title_sprite_records();
      return;
    }
    DAT_00ff1594 = 1;
    DAT_00ff1593 = DAT_00ff1593 + '\x01';
    if (DAT_00ff1593 < '\r') goto LAB_0001bd50;
    DAT_00ffe058 = 2;
    DAT_00ffe056 = 0x1340;
    DAT_00ffe054 = 0xff0;
    DAT_00ffe05c = 0xf;
    DAT_00ffe052 = 0x3c;
    DAT_00ffe05a = 0;
  }
  DAT_00ffe052 = DAT_00ffe052 + -1;
  if (-1 < DAT_00ffe052) {
    DAT_00ffe05c = DAT_00ffe05c + -1;
    if (DAT_00ffe05c < 0) {
      DAT_00ffe05c = 0xf;
      DAT_00ffe05a = DAT_00ffe05a + 1;
      if (3 < DAT_00ffe05a) goto LAB_0001bdb8;
    }
    DAT_00ffe056 = 0x1340 - (short)((uint)(*(short *)((int)&DAT_00015e56 +
                                                     (int)(short)((0x3c - DAT_00ffe052) * 3 & 0xfe))
                                          * 0x1600) >> 0x10);
    DAT_00ffe054 = DAT_00ffe054 + -10;
    build_title_sprite_records();
    return;
  }
LAB_0001bdb8:
  DAT_00ffe058 = 0;
  _DAT_00ffe0d6 = 0;
  if (_DAT_00ffe0c8 != 0) {
    _DAT_00ffe0f4 = 1;
  }
  refresh_menu_selection_or_prepare();
  return;
}

