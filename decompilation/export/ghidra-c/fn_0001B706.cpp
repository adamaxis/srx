// Ghidra headless export
// Entry: 0x0001B706
// Name: FUN_0001b706
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00017E78 FUN_00017e78
//   0x0001A908 FUN_0001a908


void FUN_0001b706(void)

{
  if (DAT_00ffdf84 == '\0') {
    DAT_00ffdf84 = -1;
    DAT_00ffe074 = FUN_00017e78();
    FUN_0001a908();
  }
  return;
}


