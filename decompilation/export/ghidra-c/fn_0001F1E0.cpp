// Ghidra headless export
// Entry: 0x0001F1E0
// Name: FUN_0001f1e0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0001f1e0(void)

{
  char cVar1;
  
  cVar1 = DAT_00fffbfb + -8;
  DAT_00fffbfb = DAT_00fffbfb + '\x01';
  if (-1 < cVar1) {
    DAT_00fffbfb = '\b';
  }
  return;
}


