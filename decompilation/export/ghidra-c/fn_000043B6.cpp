// Ghidra headless export
// Entry: 0x000043B6
// Name: FUN_000043b6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00004466 FUN_00004466
//   0x0000A800 FUN_0000a800
//   0x0001DBAA FUN_0001dbaa
//   0x0005BA4C thunk_FUN_0005ba50


void FUN_000043b6(void)

{
  if (DAT_00ffe186 == '\x02') {
    thunk_FUN_0005ba50();
  }
  FUN_0000a800();
  if (DAT_00ffe124 != 0) {
    FUN_00004466();
    DAT_00ffe124 = 0;
  }
  if (DAT_00ffe120 != (undefined *)0x0) {
    if (DAT_00ffe120 == &DAT_0001e628) {
      DAT_00ffe12e = &UNK_00053ffc;
      FUN_0001dbaa();
      DAT_00ffe177 = 4;
    }
    else if (DAT_00ffe120 == &UNK_0001e5c4) {
      DAT_00ffe12e = &UNK_00054286;
      FUN_0001dbaa();
      DAT_00ffe177 = 4;
    }
    FUN_00004466();
    DAT_00ffe120 = (undefined *)0x0;
    FUN_00004466();
    return;
  }
  FUN_00004466();
  return;
}


