// Ghidra headless export
// Entry: 0x00017C98
// Name: FUN_00017c98
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00017c98(void)

{
  char in_D0b;
  short sVar1;
  short sVar2;
  
  sVar1 = 0;
  sVar2 = 4;
  do {
    if (in_D0b == (&DAT_00fffbd8)[sVar1]) {
      return;
    }
    sVar1 = sVar1 + 1;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


