// Ghidra headless export
// Entry: 0x00001C1E
// Name: FUN_00001c1e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000010D6 FUN_000010d6
//   0x00001DF6 FUN_00001df6
//   0x000530EA FUN_000530ea


void FUN_00001c1e(void)

{
  if (DAT_00ffdd5a == 1) {
    DAT_00ffdd5c = DAT_00ffdd5c - 1;
    if ((short)DAT_00ffdd5c < 0) {
      DAT_00ffdd5a = 2;
      FUN_000530ea();
      FUN_00001df6();
    }
    else if ((DAT_00ffdd5c & 0x1f) == 0) {
      FUN_000010d6();
      DAT_00ffef14 = DAT_00ffef14 | 1;
      return;
    }
  }
  return;
}


