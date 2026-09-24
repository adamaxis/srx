// Ghidra headless export
// Entry: 0x00059172
// Name: FUN_00059172
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 FUN_00000bd2
//   0x00000BF2 FUN_00000bf2
//   0x00000D94 FUN_00000d94
//   0x0000195C FUN_0000195c
//   0x0000C0DA FUN_0000c0da
//   0x0000C376 FUN_0000c376
//   0x0000C3E6 FUN_0000c3e6
//   0x00059360 FUN_00059360


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00059172(void)

{
  undefined1 uVar1;
  
  FUN_0000c0da();
  DAT_00fff0dc = 0;
  do {
    uVar1 = 0;
    FUN_0000c376();
    while( true ) {
      FUN_00000bd2();
      FUN_00000bf2();
      FUN_0000195c();
      if (((DAT_00ffdd22._0_1_ & 1) != 0) || ((DAT_00ffdd22._0_1_ & 2) != 0)) break;
      if (((DAT_00ffdd22._0_1_ & 0x40) != 0) ||
         (((DAT_00ffdd22._0_1_ & 0x10) != 0 || ((DAT_00ffdd22._0_1_ & 0x20) != 0)))) {
        FUN_0000c3e6();
        FUN_00059360();
        if ((bool)uVar1) {
          DAT_00fff0dc = 0xff;
        }
        else {
          _DAT_00ffe108 = 0xffff;
        }
        FUN_0000c0da();
        FUN_00000d94();
        return;
      }
    }
  } while( true );
}


