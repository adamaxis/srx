// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000043B6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000043b6(void)

{
  if (DAT_00ffe186 == '\x02') {
    thunk_FUN_0005ba50();
  }
  FUN_0000a800();
  if (_DAT_00ffe124 != 0) {
    FUN_00004466();
    _DAT_00ffe124 = 0;
  }
  if (_DAT_00ffe120 != 0) {
    if (_DAT_00ffe120 == 0x1e628) {
      _DAT_00ffe12e = 0x53ffc;
      FUN_0001dbaa();
      DAT_00ffe177 = 4;
    }
    else if (_DAT_00ffe120 == 0x1e5c4) {
      _DAT_00ffe12e = 0x54286;
      FUN_0001dbaa();
      DAT_00ffe177 = 4;
    }
    FUN_00004466();
    _DAT_00ffe120 = 0;
    FUN_00004466();
    return;
  }
  FUN_00004466();
  return;
}


