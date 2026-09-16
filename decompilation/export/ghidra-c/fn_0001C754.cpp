// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001C754


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001c754(void)

{
  int extraout_A0;
  int extraout_A1;
  int extraout_A1_00;
  
  if (_DAT_00ffe084 != 0) {
    FUN_0001b7d0();
    *(undefined **)(extraout_A0 + 8) = &DAT_0001c2ae;
    *(undefined **)(extraout_A0 + 4) = &DAT_0001c2a8;
    return;
  }
  if ((DAT_00ff1592 & 1) != 0) {
    return;
  }
  if (_DAT_00ff1584 == &PTR_DAT_0001c36a) {
    if ((DAT_00ff158d == '\x03') && (cRam00ff1583 == '\x03')) {
      if ((DAT_00ff1592 & 0x40) == 0) {
        if (sRam00ffe010 == 0) {
          if (DAT_00fff0b7 < '\0') {
            _DAT_00ffe060 = 0x134;
            _DAT_00ff1584 = (undefined **)0x1c27c;
            DAT_00ff158d = '\0';
code_r0x0001c826:
            FUN_000530de();
            _DAT_00ffe062 = 0x102;
            _DAT_00ffe05e = 0xffff;
            _DAT_00ffe064 = 0;
            _DAT_00ffe066 = 3;
            return;
          }
          if (DAT_00fff0b7 != '\0') {
            _DAT_00ffe06c = -10;
            _DAT_00ffe06a = 0x1340;
code_r0x0001c9b2:
            FUN_000530de();
            _DAT_00ffe068 = 0x1020;
            _DAT_00ffe06e = 0xffff;
            _DAT_00ffe070 = 0x1e;
            _DAT_00ffe072 = 0;
            if (_DAT_00ffe0e2 != 0) {
              if (_DAT_00ffe06c < 0) {
                _DAT_00ffe06c = _DAT_00ffe06c + -8;
              }
              else {
                _DAT_00ffe06c = _DAT_00ffe06c + 8;
              }
            }
            return;
          }
        }
        func_0x0001cb6a();
        if (_DAT_00ffe0e2 == 0) {
          return;
        }
        *(short *)(extraout_A1_00 + 0xe) = *(short *)(extraout_A1_00 + 0xe) + -6;
        return;
      }
code_r0x0001cd14:
      if (DAT_00ff15cc != '\0') {
        return;
      }
      DAT_00ff15cc = 0xff;
      uRam00ff15d0 = 0;
      uRam00ff15cf = 0;
      uRam00ff15cb = 0;
      uRam00ff15ca = 0x14;
      uRam00ff15d1 = 0x3c;
      uRam00ff15c6 = 0x1400;
      uRam00ff15c8 = 0x1070;
      uRam00ff15d2 = 0xfff8;
      uRam00ff15d4 = 0xfff2;
      return;
    }
    if (DAT_00ff158d != '\x05') {
      return;
    }
  }
  else if (_DAT_00ff1588 == &PTR_DAT_0001c350) {
    if ((DAT_00ff158c == '\x03') && (DAT_00ff1582 == '\x03')) {
      if ((DAT_00ff1592 & 0x40) == 0) {
        if (sRam00ffe010 == 0) {
          if (DAT_00fff0b7 < '\0') {
            _DAT_00ff1588 = (undefined **)0x1c282;
            _DAT_00ffe060 = 0x10c;
            DAT_00ff158c = '\0';
            goto code_r0x0001c826;
          }
          if (DAT_00fff0b7 != '\0') {
            _DAT_00ffe06a = 0x10c0;
            _DAT_00ffe06c = 10;
            goto code_r0x0001c9b2;
          }
        }
        func_0x0001cb6a();
        if (_DAT_00ffe0e2 == 0) {
          return;
        }
        *(short *)(extraout_A1 + 0xe) = *(short *)(extraout_A1 + 0xe) + -6;
        return;
      }
      goto code_r0x0001cd14;
    }
    if (DAT_00ff158c != '\x05') {
      return;
    }
  }
  if (((_DAT_00ffe076 < 3) &&
      ((((DAT_00ffdd22 & 0x40) != 0 || ((DAT_00ffdd22 & 0x10) != 0)) || ((DAT_00ffdd22 & 0x20) != 0)
       ))) && (_DAT_00ffe082 == 0)) {
    FUN_00019976();
    return;
  }
  return;
}


