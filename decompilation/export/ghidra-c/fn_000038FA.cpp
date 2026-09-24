// Ghidra headless export
// Entry: 0x000038FA
// Name: FUN_000038fa
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003674 FUN_00003674
//   0x000036FC FUN_000036fc
//   0x000037EC FUN_000037ec


void FUN_000038fa(void)

{
  short sVar1;
  
  sVar1 = 2;
  do {
    FUN_00003674();
    FUN_000037ec();
    FUN_000036fc();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


