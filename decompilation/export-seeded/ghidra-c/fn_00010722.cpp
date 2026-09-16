// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00010722


void FUN_00010722(void)

{
  FUN_000107ce();
  do {
    while( true ) {
      while( true ) {
        FUN_00000bd2();
        FUN_00000bf2();
        FUN_0000195c();
        FUN_0000fdfe();
        if ((DAT_00ffdd22 & 1) != 0) {
          FUN_000107ce();
          return;
        }
        if ((DAT_00ffdd22 & 8) == 0) break;
        FUN_000107ce();
      }
      if ((DAT_00ffdd22 & 4) == 0) break;
      FUN_000107ce();
    }
  } while ((((DAT_00ffdd22 & 0x40) == 0) && ((DAT_00ffdd22 & 0x10) == 0)) &&
          ((DAT_00ffdd22 & 0x20) == 0));
  FUN_000107ce();
  return;
}


