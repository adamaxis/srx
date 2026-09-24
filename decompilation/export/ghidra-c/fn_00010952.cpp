// Ghidra headless export
// Entry: 0x00010952
// Name: FUN_00010952
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000E34 FUN_00000e34
//   0x00000F1C FUN_00000f1c
//   0x00000F3C FUN_00000f3c
//   0x00000FFE FUN_00000ffe
//   0x0000102A FUN_0000102a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00010952(void)

{
  FUN_00000ffe();
  FUN_0000102a();
  wait_for_vblank();
  FUN_00000e34();
  _DAT_00ffdd08 = 0;
  _DAT_00ffdd0c = 0;
  FUN_00000f1c();
  FUN_00000f3c();
  return;
}


