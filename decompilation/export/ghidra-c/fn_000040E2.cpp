// Ghidra headless export
// Entry: 0x000040E2
// Name: FUN_000040e2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003DDE FUN_00003dde
//   0x00004000 FUN_00004000
//   0x000041F0 FUN_000041f0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

short FUN_000040e2(void)

{
  short in_D0w;
  short sVar1;
  
  if (in_D0w < 0) {
    sVar1 = (DAT_00ffddc6 >> 4) + 8;
    if (sVar1 <= DAT_00ffddce) {
      FUN_000041f0();
      sVar1 = DAT_00ffdd08 + -0x148;
      FUN_00004000();
      FUN_00003dde();
      _DAT_00ffdf58 = 0x8000;
      sVar1 = FUN_00003dde(sVar1);
      _DAT_00ffdf58 = 0;
    }
  }
  else {
    sVar1 = (DAT_00ffddc6 >> 4) - 8;
    if (-1 < sVar1) {
      FUN_000041f0();
      sVar1 = DAT_00ffdd08 + 8;
      FUN_00004000();
      FUN_00003dde();
      _DAT_00ffdf58 = 0x8000;
      sVar1 = FUN_00003dde(sVar1);
      _DAT_00ffdf58 = 0;
    }
  }
  return sVar1;
}


