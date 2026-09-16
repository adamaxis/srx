// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001F830


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001f830(void)

{
  if ((DAT_00fffbf8 == '\0') && (DAT_00fffbf9 == '\0')) {
    DAT_00fff0b6 = 0xb;
    if ((((DAT_00fff0d5 == '\0') && ((_DAT_00fffc30 & 8) == 0)) && ((_DAT_00fffc1e & 0x800000) != 0)
        ) && ((_DAT_00fffc1e & 0x1000000) != 0)) {
      if ((_DAT_00fffc1e & 0x2000000) == 0) {
        _DAT_00fffc34 = _DAT_00fffc34 & 0xfffbffff | 0x80000;
        _DAT_00ffdd64 = 0x19;
        FUN_0002196c();
      }
      else if (((_DAT_00fffc1e & 0x20000000) == 0) && ((_DAT_00fffc22 & 1) != 0)) {
        _DAT_00fffc22 = _DAT_00fffc22 & 0xfffffffe;
        _DAT_00fffc2a = _DAT_00fffc2a | 0x200;
        _DAT_00fffc34 = _DAT_00fffc34 & 0xffefffff;
        _DAT_00fffc38 = _DAT_00fffc38 & 0xfffff7f3;
        _DAT_00fffc1e = _DAT_00fffc1e | 0x20000000;
      }
    }
  }
  else if ((DAT_00fffbf8 == '\x02') && (DAT_00fffbf9 == '\x01')) {
    DAT_00fff0b6 = 8;
  }
  else if ((DAT_00fffbf8 == '\x01') && (DAT_00fffbf9 == '\x06')) {
    DAT_00fff0b6 = 9;
  }
  else {
    if (DAT_00fffbf8 != '\x04') {
      return;
    }
    if (DAT_00fffbf9 != '\x04') {
      return;
    }
    DAT_00fff0b6 = 10;
  }
  DAT_00fff0d5 = '\0';
  FUN_0001f246();
  if (DAT_00fff0d5 != '\0') {
    DAT_00fff0d5 = 0;
    FUN_0000c928();
    return;
  }
  return;
}


