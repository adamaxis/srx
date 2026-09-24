// Ghidra headless export
// Entry: 0x0001EEDC
// Name: dispatch_world_effect
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void dispatch_world_effect(void)

{
  DAT_00fff0d5 = 0;
  if (DAT_00fffbec == -1) {
    return;
  }
  if ((DAT_00fffc1c & 0x80) != 0) {
    return;
  }
  if (-1 < (char)((byte)DAT_00fffbec - 6)) {
    return;
  }
  DAT_00fff10c = 0;
  (**(code **)(&DAT_000f3e56 + (uint)(byte)DAT_00fffbec * 4))();
  return;
}


