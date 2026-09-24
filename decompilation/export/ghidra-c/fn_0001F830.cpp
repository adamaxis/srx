// Ghidra headless export
// Entry: 0x0001F830
// Name: FUN_0001f830
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000C928 FUN_0000c928
//   0x0001F246 FUN_0001f246
//   0x0002196C FUN_0002196c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001f830(void)

{
  if ((DAT_00fffbf8 == '\0') && (DAT_00fffbf9 == '\0')) {
    DAT_00fff0b6 = 0xb;
    if ((((DAT_00fff0d5 == '\0') && ((DAT_00fffc30 & 8) == 0)) && ((DAT_00fffc1e & 0x800000) != 0))
       && ((DAT_00fffc1e & 0x1000000) != 0)) {
      if ((DAT_00fffc1e & 0x2000000) == 0) {
        DAT_00fffc34 = DAT_00fffc34 & 0xfffbffff | 0x80000;
        _DAT_00ffdd64 = 0x19;
        FUN_0002196c();
      }
      else if (((DAT_00fffc1e & 0x20000000) == 0) && ((DAT_00fffc22 & 1) != 0)) {
        DAT_00fffc22 = DAT_00fffc22 & 0xfffffffe;
        DAT_00fffc2a = DAT_00fffc2a | 0x200;
        DAT_00fffc34 = DAT_00fffc34 & 0xffefffff;
        DAT_00fffc38 = DAT_00fffc38 & 0xfffff7f3;
        DAT_00fffc1e = DAT_00fffc1e | 0x20000000;
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


