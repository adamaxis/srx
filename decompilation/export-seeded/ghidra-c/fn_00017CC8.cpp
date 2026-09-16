// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00017CC8


undefined8 FUN_00017cc8(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  
  DAT_00fff0de = DAT_00fff0de + -1;
  if (DAT_00fff0de < '\0') {
    DAT_00fff0de = '\x01';
    DAT_00fff0df = DAT_00fff0df + '\x01';
    if ('\x03' < DAT_00fff0df) {
      DAT_00fff0df = '\0';
    }
    FUN_00001728();
    if (DAT_00fff0df == '\0') {
      FUN_00017d3e();
    }
  }
  return CONCAT44(in_D0,in_D1);
}


