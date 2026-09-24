// Ghidra headless export
// Entry: 0x00013874
// Name: FUN_00013874
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001438C FUN_0001438c
//   0x00014446 FUN_00014446


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00013874(void)

{
  DAT_00ffe175 = 0;
  if (_DAT_00ffdfe8 == 0) {
    if ((DAT_00ffdf88 & 0x3f) == 0xb) {
      FUN_0001438c();
    }
    else if ((DAT_00ffdf88 & 0x3f) == 0xf) {
      FUN_00014446();
    }
  }
  else {
    FUN_0001438c();
    FUN_00014446();
  }
  return;
}


