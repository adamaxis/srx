// Ghidra headless export
// Entry: 0x00002900
// Name: FUN_00002900
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001610 FUN_00001610
//   0x0000313A FUN_0000313a
//   0x00004388 FUN_00004388
//   0x00004B6C FUN_00004b6c
//   0x00005B7E FUN_00005b7e
//   0x0000EE10 FUN_0000ee10
//   0x00013874 FUN_00013874
//   0x000164A8 FUN_000164a8
//   0x000164DC FUN_000164dc
//   0x00016DEE FUN_00016dee
//   0x00016E5C FUN_00016e5c
//   0x0001A310 FUN_0001a310
//   0x0001A322 FUN_0001a322
//   0x0001E04A FUN_0001e04a
//   0x000530EA FUN_000530ea
//   0x0005BA50 FUN_0005ba50
//   0x0005D260 FUN_0005d260


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00002900(void)

{
  if (DAT_00ffe172 != '\0') {
    DAT_00ffe172 = '\0';
    FUN_000530ea();
  }
  if (_DAT_00ffdf8a != 0) {
    if (DAT_00ffdca0 != 0) {
      FUN_00004b6c();
    }
    DAT_00ffe16f = 0;
    if (DAT_00ffe191 != '\0') {
      DAT_00ffe191 = '\0';
      FUN_0001e04a();
    }
    FUN_0001a310();
    FUN_0001a322();
    FUN_00016dee();
    FUN_00004388();
    if (DAT_00ffe186 != '\x03') {
      FUN_0005d260();
    }
    if (DAT_00ffe171 == '\0') {
      FUN_0000313a();
    }
    else {
      DAT_00ffe171 = '\0';
      _DAT_00ffdf8a = 0;
    }
    if (DAT_00ffdf40 == 2) {
      FUN_00016e5c();
    }
  }
  FUN_0000ee10();
  if ('\x01' < DAT_00ffe186) {
    FUN_0005ba50();
  }
  FUN_000164dc();
  FUN_000164a8();
  FUN_00013874();
  if ((DAT_00ffde22 & 1) != 0) {
    FUN_00001610();
    DAT_00ffddde = &DAT_00ff6700;
    DAT_00ffdde6 = &DAT_00ff6700;
  }
  FUN_00005b7e();
  return;
}


