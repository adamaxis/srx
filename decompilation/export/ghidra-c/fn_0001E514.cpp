// Ghidra headless export
// Entry: 0x0001E514
// Name: FUN_0001e514
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001A304 FUN_0001a304


void FUN_0001e514(void)

{
  byte bVar1;
  int in_A0;
  
  bVar1 = *(byte *)(in_A0 + 0xb0);
  *(byte *)(in_A0 + 0xb0) = bVar1 & 0x7f;
  if ((bVar1 & 0x80) != 0) {
    DAT_00fffc3c = 0;
  }
  *(byte *)(in_A0 + 0x10) = *(byte *)(in_A0 + 0x10) & 0xfe;
  if ((*(byte *)(in_A0 + 0x10) & 8) == 0) {
    FUN_0001a304();
  }
  return;
}


