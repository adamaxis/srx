// Ghidra headless export
// Entry: 0x0001B218
// Name: FUN_0001b218
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000013BE FUN_000013be
//   0x000061CC FUN_000061cc


void FUN_0001b218(void)

{
  DAT_00ffe0fa = DAT_00ffe0fa + -1;
  if (DAT_00ffe0fa < 0) {
    DAT_00ffe0fa = 3;
    DAT_00ffe0f8 = DAT_00ffe0f8 + 1;
    if (5 < DAT_00ffe0f8) {
      DAT_00ffe0f8 = 0;
    }
  }
  FUN_000061cc();
  FUN_000013be();
  return;
}


