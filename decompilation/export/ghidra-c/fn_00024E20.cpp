// Ghidra headless export
// Entry: 0x00024E20
// Name: dispatch_indexed_handler
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void dispatch_indexed_handler(void)

{
  short in_D0w;
  
  (*(dispatch_indexed_handler + *(short *)(&DAT_00024e3a + (short)(in_D0w * 2))))();
  return;
}


