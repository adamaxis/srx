// Ghidra headless export
// Entry: 0x00055F02
// Name: FUN_00055f02
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00055f02(void)

{
  short sVar1;
  int in_A0;
  
  sVar1 = 6;
  do {
    if (*(char *)(in_A0 + 0x66 + (int)sVar1) == '\x18') {
      return;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


