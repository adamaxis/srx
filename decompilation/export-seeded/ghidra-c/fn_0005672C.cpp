// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005672C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005672c(void)

{
  if (DAT_00ffe18a != '\0') {
    return;
  }
  if (_DAT_00ffddc4 != 0x14) {
    if (_DAT_00ffdf40 == 1) {
      _DAT_00ffdf90 = _DAT_00ffdf90 + -1;
      if (_DAT_00ffdf90 < 0) {
        _DAT_00ffdf90 = 6;
        _DAT_00ffdf92 = _DAT_00ffdf92 + 1;
        if (3 < _DAT_00ffdf92) {
          _DAT_00ffdf92 = 0;
        }
        _DAT_00c00004 = 0;
        _DAT_00c00000 = *(short *)((int)&DAT_0018ad70 + (int)(short)(_DAT_00ffdf92 << 1));
      }
    }
    else if (_DAT_00ffdf40 == 2) {
      if ((_DAT_00ffdf88 & 0xf) == 0xf) {
        if (_DAT_00ffdf90 < 0) {
          _DAT_00ffdf92 = _DAT_00ffdf92 - 1 & 7;
          if (_DAT_00ffdf92 == 7) {
            _DAT_00ffdf90 = 0;
            _DAT_00ffdf92 = 0;
          }
        }
        else {
          _DAT_00ffdf92 = _DAT_00ffdf92 + 1 & 7;
          if (_DAT_00ffdf92 == 0) {
            _DAT_00ffdf90 = -1;
            _DAT_00ffdf92 = 7;
          }
        }
        _DAT_00c00000 = _DAT_00ffdf92 << 9;
        _DAT_00c00004 = 0;
      }
    }
    else if (_DAT_00ffdf40 == 0) {
      if ((((_DAT_00ffddc4 == 6) || (_DAT_00ffddc4 == 7)) || (_DAT_00ffddc4 == 8)) ||
         (_DAT_00ffddc4 == 9)) {
        _DAT_00ffdf90 = _DAT_00ffdf90 + -1;
        if (_DAT_00ffdf90 < 0) {
          _DAT_00ffdf90 = 0xc;
          _DAT_00ffdf92 = _DAT_00ffdf92 + 1;
          if (2 < _DAT_00ffdf92) {
            _DAT_00ffdf92 = 0;
          }
          _DAT_00c00004 = 0;
          _DAT_00c00000 = *(short *)((int)&DAT_0018ad7e + (int)(short)(_DAT_00ffdf92 << 3));
        }
      }
      else if (_DAT_00ffddc4 == 4) {
        _DAT_00ffdf90 = _DAT_00ffdf90 + -1;
        if (_DAT_00ffdf90 < 0) {
          _DAT_00ffdf90 = 10;
          _DAT_00ffdf92 = _DAT_00ffdf92 + 1;
          if (2 < _DAT_00ffdf92) {
            _DAT_00ffdf92 = 0;
          }
          _DAT_00c00004 = 0;
          _DAT_00c00000 = *(undefined2 *)((int)&DAT_0018ad96 + (int)(short)(_DAT_00ffdf92 << 3));
          return;
        }
      }
      else if (_DAT_00ffddc4 == 5) {
        _DAT_00ffdf90 = _DAT_00ffdf90 + -1;
        if (_DAT_00ffdf90 < 0) {
          _DAT_00ffdf90 = 10;
          _DAT_00ffdf92 = _DAT_00ffdf92 + 1;
          if (2 < _DAT_00ffdf92) {
            _DAT_00ffdf92 = 0;
          }
          _DAT_00c00004 = 0;
          _DAT_00c00000 = *(undefined2 *)((int)&DAT_0018ad52 + (int)(short)(_DAT_00ffdf92 << 1));
          return;
        }
      }
      else if (_DAT_00ffddc4 == 0) {
        _DAT_00ffdf90 = _DAT_00ffdf90 + -1;
        if (_DAT_00ffdf90 < 0) {
          _DAT_00ffdf90 = 6;
          _DAT_00ffdf92 = _DAT_00ffdf92 + 1;
          if (5 < _DAT_00ffdf92) {
            _DAT_00ffdf92 = 0;
          }
          _DAT_00c00004 = 0;
          _DAT_00c00000 = *(short *)((int)&DAT_0018ad42 + (int)(short)(_DAT_00ffdf92 << 1));
        }
      }
      else if (((_DAT_00ffddc4 < 0x32) && (0x2b < _DAT_00ffddc4)) &&
              (_DAT_00ffdf90 = _DAT_00ffdf90 + -1, _DAT_00ffdf90 < 0)) {
        _DAT_00ffdf90 = 9;
        _DAT_00ffdf92 = _DAT_00ffdf92 + 1;
        if (3 < _DAT_00ffdf92) {
          _DAT_00ffdf92 = 0;
        }
        _DAT_00c00004 = 0;
        _DAT_00c00000 = *(short *)((int)&PTR_LAB_0018ad60 + (int)(short)(_DAT_00ffdf92 << 1));
      }
    }
  }
  return;
}


