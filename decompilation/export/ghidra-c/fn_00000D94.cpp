// Ghidra headless export
// Entry: 0x00000D94
// Name: FUN_00000d94
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 FUN_00000bd2
//   0x00000BF2 FUN_00000bf2
//   0x0000195C FUN_0000195c


void FUN_00000d94(void)

{
  do {
    FUN_00000bd2();
    FUN_00000bf2();
    FUN_0000195c();
  } while ((DAT_00ffdd22 & 0xff00) == 0);
  FUN_0000195c();
  return;
}


