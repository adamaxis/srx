// Ghidra headless export
// Entry: 0x000164F2
// Name: FUN_000164f2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_000164f2(void)

{
  short sVar1;
  short *psVar2;
  
  psVar2 = &DAT_00ffd740;
  sVar1 = 9;
  do {
    if (*psVar2 == 0) {
      return;
    }
    psVar2 = psVar2 + 0xf;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


