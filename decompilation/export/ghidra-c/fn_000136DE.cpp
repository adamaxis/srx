// Ghidra headless export
// Entry: 0x000136DE
// Name: initialize_scene_mode_state
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00053126 prepare_mode_entry_service


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void initialize_scene_mode_state(void)

{
  DAT_00ffddc4 = prepare_mode_entry_service();
  DAT_00ffdf40 = 0;
  DAT_00fff0a0 = 0xff;
  _DAT_00ffdf8e = 0xffff;
  if (DAT_00ffddc4 == 6) {
    DAT_00fffc10 = 0;
    _DAT_00ffe10e = 1;
  }
  _DAT_00fffc0e = (ushort)(DAT_00ffddc4 == 6);
  return;
}


