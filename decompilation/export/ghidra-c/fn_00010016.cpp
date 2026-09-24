// Ghidra headless export
// Entry: 0x00010016
// Name: FUN_00010016
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 FUN_00000bd2
//   0x00000BF2 FUN_00000bf2
//   0x0000195C FUN_0000195c
//   0x0000FDFE FUN_0000fdfe
//   0x000100A8 FUN_000100a8
//   0x00010172 FUN_00010172


void FUN_00010016(void)

{
  FUN_00010172();
  do {
    while( true ) {
      FUN_0000fdfe();
      FUN_00000bd2();
      FUN_00000bf2();
      FUN_0000195c();
      if (((DAT_00ffdd22._0_1_ & 8) == 0) && ((DAT_00ffdd22._0_1_ & 4) == 0)) break;
      FUN_000100a8();
    }
  } while (((DAT_00ffdd22._0_1_ & 2) == 0) &&
          ((((DAT_00ffdd22._0_1_ & 0x40) == 0 && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) &&
           ((DAT_00ffdd22._0_1_ & 0x20) == 0))));
  FUN_00010172();
  return;
}


