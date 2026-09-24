// Ghidra headless export
// Entry: 0x0001C3FA
// Name: FUN_0001c3fa
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001C506 FUN_0001c506


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001c3fa(void)

{
  if (_DAT_00ffdf8c != 0) {
    FUN_0001c506();
    return;
  }
  if ((DAT_00ff158c < (char)*DAT_00ff1588) &&
     (DAT_00ff1582._0_1_ = DAT_00ff1582._0_1_ + -1, DAT_00ff1582._0_1_ < '\0')) {
    DAT_00ff1582._0_1_ = '\x04';
    if ((char)(DAT_00ff158c + '\x01') <= (char)*DAT_00ff1588) {
      DAT_00ff158c = DAT_00ff158c + '\x01';
    }
  }
  if ((DAT_00ff158d < (char)*DAT_00ff1584) &&
     (DAT_00ff1582._1_1_ = (char)DAT_00ff1582 + -1, (char)DAT_00ff1582 < '\0')) {
    DAT_00ff1582._1_1_ = '\x04';
    if ((char)(DAT_00ff158d + '\x01') <= (char)*DAT_00ff1584) {
      DAT_00ff158d = DAT_00ff158d + '\x01';
    }
  }
  FUN_0001c506();
  return;
}


