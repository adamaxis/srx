// Ghidra headless export
// Entry: 0x0001E04A
// Name: FUN_0001e04a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001DBAA FUN_0001dbaa
//   0x0001DF00 FUN_0001df00
//   0x0001E0DC FUN_0001e0dc
//   0x000530DE FUN_000530de


void FUN_0001e04a(void)

{
  if (DAT_00ffddc4 == 0x14) {
    FUN_0001dbaa();
  }
  else if ((DAT_00ffdf40 == 2) && (DAT_00ffe11e = 0x960, DAT_00ffe16f == '\0')) {
    DAT_00ffe16f = -1;
    FUN_000530de();
    FUN_0001df00();
    FUN_0001e0dc();
  }
  return;
}


