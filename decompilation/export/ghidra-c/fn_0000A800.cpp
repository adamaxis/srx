// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000A800


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000a800(void)

{
  char cVar1;
  undefined4 *unaff_A3;
  
  if (DAT_00ffe180 == '\0') {
    if (DAT_00ffe182 == '\0') {
      if ((DAT_00fffbed == '\x05') && (_DAT_00ffdf40 == 2)) {
        if (DAT_00ffe183 == '\0') {
          cVar1 = FUN_00000d3a();
          DAT_00ffe183 = cVar1 + '\x01';
        }
        if ((_DAT_00ffe12c == _DAT_00fff0c8) && (_DAT_00fffc3c == 0)) {
          func_0x0005ba98();
          _DAT_00ffe110 = *unaff_A3;
          _DAT_00ffe124 = unaff_A3;
        }
      }
    }
    else {
      _DAT_00ffe124 = &DAT_0000a8cc;
      _DAT_00ffe110 = DAT_0000a8cc;
    }
  }
  else if (_DAT_00fff0c8 == 4) {
    _DAT_00ffe124 = &DAT_0000a8ae;
    _DAT_00ffe110 = DAT_0000a8ae;
  }
  return;
}


