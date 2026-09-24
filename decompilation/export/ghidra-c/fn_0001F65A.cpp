// Ghidra headless export
// Entry: 0x0001F65A
// Name: FUN_0001f65a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000CF38 FUN_0000cf38
//   0x0001F246 run_mode_exit_services


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
  run_mode_exit_services();
  if (DAT_00fff10d < '\0') {
    FUN_0000cf38();
  }
  return;
}


