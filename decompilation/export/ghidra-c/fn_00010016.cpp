// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00010016


void FUN_00010016(void)

{
  FUN_00010172();
  do {
    while( true ) {
      FUN_0000fdfe();
      FUN_00000bd2();
      FUN_00000bf2();
      FUN_0000195c();
      if (((DAT_00ffdd22 & 8) == 0) && ((DAT_00ffdd22 & 4) == 0)) break;
      FUN_000100a8();
    }
  } while (((DAT_00ffdd22 & 2) == 0) &&
          ((((DAT_00ffdd22 & 0x40) == 0 && ((DAT_00ffdd22 & 0x10) == 0)) &&
           ((DAT_00ffdd22 & 0x20) == 0))));
  FUN_00010172();
  return;
}


