// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001FD80


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001fd80(void)

{
  short sVar1;
  
  if ((DAT_00fffbf8 == '\x06') && (DAT_00fffbf9 == '\0')) {
    _DAT_00fffc30 = _DAT_00fffc30 & 0xfffffdff;
    FUN_0001f246();
    return;
  }
  if ((DAT_00fffbf8 == '\a') && (DAT_00fffbf9 == '\0')) {
    if ((_DAT_00fffc1e & 0x1000000) == 0) {
      _DAT_00fffc1e = _DAT_00fffc1e | 0x1000000;
    }
    else if ((_DAT_00fffc1e & 0x2000000) != 0) {
      FUN_00059bcc();
    }
code_r0x0001fe7a:
    FUN_0001f246();
    return;
  }
  if ((DAT_00fffbf8 == '\t') && (DAT_00fffbf9 == '\0')) goto code_r0x0001fe7a;
  DAT_00fff0d6 = 0;
  if (DAT_00fffbf8 == '\0') {
    if ((((DAT_00fffbf9 == '\x01') || (DAT_00fffbf9 == '\x02')) || (DAT_00fffbf9 == '\x04')) ||
       (DAT_00fffbf9 == '\x05')) goto LAB_0001fed2;
    if (DAT_00fffbf9 == '\x06') {
      if (DAT_00fff0d5 == '\0') {
        DAT_00fff0d6 = 0xff;
      }
      goto LAB_0001fed2;
    }
    if (DAT_00fffbf9 == '\a') goto LAB_0001fed2;
    if ((DAT_00fffbf9 != '\b') && (DAT_00fffbf9 != '\t')) goto LAB_00020054;
LAB_0002003c:
    DAT_00fff0d6 = 0xff;
  }
  else {
LAB_00020054:
    if (DAT_00fffbf8 == '\x04') {
      if (DAT_00fffbf9 == '\x01') {
        if (DAT_00fff0d5 == '\0') {
          DAT_00fff0d6 = 0xff;
        }
        goto LAB_0001fed2;
      }
      if ((DAT_00fffbf9 == '\x02') || (DAT_00fffbf9 == '\x05')) goto LAB_0001fed2;
    }
    if (DAT_00fffbf8 == '\x03') {
      if ((((DAT_00fffbf9 == '\0') || (DAT_00fffbf9 == '\x02')) || (DAT_00fffbf9 == '\x03')) ||
         (DAT_00fffbf9 == '\x04')) goto LAB_0001fed2;
      if (DAT_00fffbf9 == '\x05') {
        if (DAT_00fff0d5 == '\0') {
          DAT_00fff0d6 = 0xff;
        }
        goto LAB_0001fed2;
      }
      if (DAT_00fffbf9 == '\x06') goto LAB_0001fed2;
      if (((DAT_00fffbf9 == '\a') || (DAT_00fffbf9 == '\b')) || (DAT_00fffbf9 == '\t'))
      goto LAB_0002003c;
    }
    if (DAT_00fffbf8 == '\x01') {
      if (DAT_00fffbf9 == '\x02') goto LAB_0001fed2;
      if (DAT_00fffbf9 == '\0') {
        if (DAT_00fff0d5 == '\0') {
          sVar1 = FUN_00000d3a();
          if ((short)(sVar1 + -0x4b) < 0) {
            DAT_00fff0d6 = 0xff;
          }
          else {
            FUN_0001e1fc();
          }
        }
        goto LAB_0001fed2;
      }
      if ((DAT_00fffbf9 == '\x04') || (DAT_00fffbf9 == '\t')) goto LAB_0001fed2;
      if ((DAT_00fffbf9 == '\b') || (DAT_00fffbf9 == '\n')) goto LAB_0002003c;
    }
    if (DAT_00fffbf8 == '\x02') {
      if ((DAT_00fffbf9 == '\0') || (DAT_00fffbf9 == '\x03')) goto LAB_0001fed2;
      if (DAT_00fffbf9 == '\x04') {
        if (DAT_00fff0d5 == '\0') {
          DAT_00fff0d6 = 0xff;
        }
        goto LAB_0001fed2;
      }
      if (DAT_00fffbf9 == '\x05') {
        if (DAT_00fff0d5 == '\0') {
          DAT_00fff0d6 = 0xff;
        }
        goto LAB_0001fed2;
      }
      if ((DAT_00fffbf9 == '\a') || (DAT_00fffbf9 == '\t')) goto LAB_0001fed2;
      if ((DAT_00fffbf9 == '\b') || (DAT_00fffbf9 == '\n')) goto LAB_0002003c;
    }
    if ((DAT_00fffbf8 != '\x05') || ((DAT_00fffbf9 != '\x01' && (DAT_00fffbf9 != '\x06')))) {
      return;
    }
  }
LAB_0001fed2:
  FUN_0001f246();
  DAT_00fff0d5 = 0;
  return;
}


