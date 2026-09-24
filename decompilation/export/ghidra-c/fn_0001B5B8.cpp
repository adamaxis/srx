// Ghidra headless export
// Entry: 0x0001B5B8
// Name: FUN_0001b5b8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001B7D0 FUN_0001b7d0
//   0x000530DE FUN_000530de


void FUN_0001b5b8(void)

{
  int extraout_A0;
  
  FUN_000530de();
  FUN_0001b7d0();
  *(undefined **)(extraout_A0 + 8) = &DAT_0001c330;
  *(undefined **)(extraout_A0 + 4) = &DAT_0001c33a;
  return;
}


