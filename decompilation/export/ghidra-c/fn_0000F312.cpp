// Ghidra headless export
// Entry: 0x0000F312
// Name: FUN_0000f312
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 FUN_00000bd2
//   0x00000BF2 FUN_00000bf2
//   0x0000195C FUN_0000195c
//   0x0000E660 FUN_0000e660


void FUN_0000f312(void)

{
  do {
    FUN_00000bd2();
    FUN_00000bf2();
    FUN_0000195c();
    FUN_0000e660();
  } while ((DAT_00ffdd22 & 0xff00) == 0);
  FUN_0000195c();
  return;
}


