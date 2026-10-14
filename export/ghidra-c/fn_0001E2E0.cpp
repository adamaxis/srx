// Ghidra headless export
// Entry: 0x0001E2E0
// Name: begin_world_activity_if_idle
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void begin_world_activity_if_idle(void)

{
  if (DAT_00ffe11c != 0) {
    return;
  }
  DAT_00ffe11c = 0x3c;
  DAT_00ffe104 = _DAT_00ffdd2c;
  return;
}


