// Ghidra headless export
// Entry: 0x00055E62
// Name: FUN_00055e62
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00055e62(void)

{
  short sVar1;
  int in_A0;
  
  if ((*(byte *)(in_A0 + 0x11) & 0x20) != 0) {
    return;
  }
  sVar1 = 7;
  do {
    if (*(char *)(in_A0 + 0x66) == '\'') {
      return;
    }
    in_A0 = in_A0 + 1;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


