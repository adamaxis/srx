// Ghidra headless export
// Entry: 0x00000436
// Name: FUN_00000436
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00000436(void)

{
  byte in_D0b;
  byte in_D1b;
  char *in_A0;
  ushort *unaff_A4;
  int *unaff_A5;
  
  *unaff_A5 = (uint)in_D0b * 0x20000 + (uint)in_D1b * 0x800000 + 0x40000003;
  for (; *in_A0 != '\0'; in_A0 = in_A0 + 1) {
    *unaff_A4 = (ushort)(byte)(*in_A0 - 0x20);
  }
  return;
}


