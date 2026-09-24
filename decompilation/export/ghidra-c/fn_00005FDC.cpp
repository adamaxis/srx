// Ghidra headless export
// Entry: 0x00005FDC
// Name: FUN_00005fdc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00005fdc(void)

{
  short sVar1;
  int in_A0;
  
  _DAT_00ffdd3e = *(undefined2 *)(in_A0 + 0x1e);
  DAT_00ffdd3c = *(short *)(in_A0 + 0x1a);
  sVar1 = *(short *)(in_A0 + 0x18) + DAT_00ffdd3c;
  if (*(short *)(in_A0 + 0x1c) <= sVar1) {
    *(short *)(in_A0 + 0x1e) = *(short *)(in_A0 + 0x1e) + 1;
    *(byte *)(in_A0 + 0x11) = *(byte *)(in_A0 + 0x11) | 1;
    sVar1 = sVar1 - *(short *)(in_A0 + 0x1c);
  }
  *(short *)(in_A0 + 0x1a) = sVar1;
  return;
}


