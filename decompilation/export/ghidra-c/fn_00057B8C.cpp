// Ghidra headless export
// Entry: 0x00057B8C
// Name: FUN_00057b8c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 FUN_00000bd2
//   0x00000BF2 FUN_00000bf2
//   0x0000195C FUN_0000195c
//   0x0000C0DA FUN_0000c0da
//   0x0000C376 FUN_0000c376
//   0x000530DE FUN_000530de


undefined8 FUN_00057b8c(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  FUN_0000c0da();
  FUN_0000c0da();
  DAT_00fff0dc = 1;
  do {
    FUN_000530de();
    FUN_0000c376();
    while( true ) {
      FUN_00000bd2();
      FUN_00000bf2();
      FUN_0000195c();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
      if (((DAT_00ffdd22._0_1_ & 0x40) != 0) || ((DAT_00ffdd22._0_1_ & 0x10) != 0)) {
        FUN_000530de();
        return CONCAT44(in_D0,in_D1);
      }
    }
  } while( true );
}


