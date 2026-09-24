// Ghidra headless export
// Entry: 0x00017EEE
// Name: FUN_00017eee
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232


void FUN_00017eee(void)

{
  DAT_00fff0b8 = 0;
  if (DAT_00fff0e4 == '\f') {
    DAT_00fff0b8 = 0xff;
  }
  DAT_00fff0e5 = DAT_00fff0e5 + -1;
  if ((DAT_00fff0e5 < '\0') && (DAT_00fff0e5 = DAT_00fff0a7, DAT_00fff0e4 != '\f')) {
    if (DAT_00fff0e4 != '\f') {
      DAT_00fff0e4 = DAT_00fff0e4 + '\x01';
    }
    FUN_00001232();
    FUN_00001232();
    return;
  }
  return;
}


