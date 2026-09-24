// Ghidra headless export
// Entry: 0x0001D1A4
// Name: FUN_0001d1a4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0001d1a4(void)

{
  int in_A0;
  
  if (*(ushort *)(in_A0 + 2) < 0xf4) {
    *(short *)(in_A0 + 2) = *(short *)(in_A0 + 2) + 2;
  }
  return;
}


