// Ghidra headless export
// Entry: 0x0001A818
// Name: begin_world_effect_descriptor_transition
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001B680 attach_behavior_descriptor_e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void begin_world_effect_descriptor_transition(void)

{
  if (_DAT_00ffe0d4 != 0) {
    return;
  }
  DAT_00ffe076 = 0;
  _DAT_00ffe082 = 1;
  if (_DAT_00ffe0f4 == 0) {
    _DAT_00ffe0f2 = 1;
    DAT_00ff1642 = 6;
    DAT_00ff1643 = 0;
    return;
  }
  _DAT_00ffe0f2 = 0;
  DAT_00ffdf7e = *(undefined4 *)(*(int *)(DAT_00ff1646 + (short)((ushort)DAT_00ff1643 << 2)) + 2);
  DAT_00ffdf76 = 0xd280;
  DAT_00ff1642 = 0;
  DAT_00ff1644 = 7;
  attach_behavior_descriptor_e();
  return;
}

