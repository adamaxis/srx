// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001F65A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001f65a(void)

{
  if ((DAT_00fffbf8 == '\0') && (DAT_00fffbf9 == '\x03')) {
    DAT_00fff10d = 'K';
  }
  else if ((DAT_00fffbf8 == '\x01') && (DAT_00fffbf9 == '\x01')) {
    DAT_00fff10d = '\x1e';
  }
  else if ((DAT_00fffbf8 == '\x02') && (DAT_00fffbf9 == '\x02')) {
    DAT_00fff10d = '\x14';
  }
  else if ((DAT_00fffbf8 == '\x03') && (DAT_00fffbf9 == '\x01')) {
    DAT_00fff10d = '2';
  }
  else {
    if (DAT_00fffbf8 != '\x04') {
      return;
    }
    if (DAT_00fffbf9 != '\0') {
      return;
    }
    DAT_00fff10d = '(';
  }
  FUN_0001f246();
  if (DAT_00fff10d < '\0') {
    FUN_0000cf38();
  }
  return;
}


