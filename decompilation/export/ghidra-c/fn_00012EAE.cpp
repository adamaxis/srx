// Ghidra headless export
// Entry: 0x00012EAE
// Name: FUN_00012eae
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C0DA FUN_0000c0da
//   0x00012FA4 FUN_00012fa4


void FUN_00012eae(void)

{
  short sVar1;
  int in_A0;
  
  sVar1 = *(byte *)(in_A0 + 0x57) - 1;
  if (sVar1 < 0) {
    if (((*(ushort *)(in_A0 + 0x90) & 8) == 0) && ((*(ushort *)(in_A0 + 0x90) & 4) == 0)) {
      FUN_0000c0da();
    }
  }
  else if ((*(char *)(in_A0 + 0x56) < '\0') && (-1 < (char)(DAT_00fff0d0 + -2))) {
    if ((char)sVar1 == '\x04') {
      FUN_0000c0da();
    }
    FUN_00012fa4();
    if ((char)(&DAT_000d4ec5)[(short)(sVar1 * 0x30)] < '\0') {
      return;
    }
  }
  FUN_00013000();
  FUN_0000c0da();
  FUN_00001232();
  return;
}


