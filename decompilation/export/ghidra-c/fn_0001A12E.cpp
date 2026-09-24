// Ghidra headless export
// Entry: 0x0001A12E
// Name: FUN_0001a12e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00019F72 FUN_00019f72


void FUN_0001a12e(void)

{
  bool bVar1;
  char cVar2;
  
  if (DAT_00fff0d2 != '\0') {
    cVar2 = DAT_00fff0d2 + -1;
    bVar1 = DAT_00fff0d2 < '\x01';
    DAT_00fff0d2 = cVar2;
    if (cVar2 == '\0' || bVar1) {
      FUN_00019f72();
    }
  }
  return;
}


