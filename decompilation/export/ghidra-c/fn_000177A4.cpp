// Ghidra headless export
// Entry: 0x000177A4
// Name: FUN_000177a4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_000177a4(void)

{
  short sVar1;
  
  sVar1 = 4;
  do {
    (&DAT_00fff0ed)[sVar1] = 6;
    (&DAT_00fff0e8)[sVar1] = 1;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


