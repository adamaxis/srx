// Ghidra headless export
// Entry: 0x000100A8
// Name: FUN_000100a8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000011C0 FUN_000011c0
//   0x0000C216 FUN_0000c216
//   0x00010156 FUN_00010156
//   0x00010172 FUN_00010172
//   0x0001055E FUN_0001055e
//   0x000105E6 FUN_000105e6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000100a8(void)

{
  short sVar1;
  short sVar2;
  short in_D1w;
  
  DAT_00fff096 = in_D1w + DAT_00fff096;
  if (DAT_00fff098 < DAT_00fff096) {
    DAT_00fff096 = DAT_00fff098;
  }
  if (DAT_00fff096 == 0) {
    DAT_00fff096 = 1;
  }
  *(char *)(DAT_00fffc04 + 0x9d) = (char)DAT_00fff096;
  FUN_0000c216();
  FUN_00010172();
  FUN_000011c0();
  sVar2 = DAT_00fff098;
  sVar1 = DAT_00fff096;
  FUN_0001055e();
  DAT_00fff098 = sVar2;
  DAT_00fff096 = sVar1;
  FUN_00010156();
  FUN_000105e6();
  return;
}


