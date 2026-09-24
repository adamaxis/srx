// Ghidra headless export
// Entry: 0x0001CA00
// Name: FUN_0001ca00
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ca00(void)

{
  DAT_00ffe06c = -10;
  DAT_00ffe06a = 0x1340;
  FUN_000530de();
  DAT_00ffe068 = 0x1020;
  _DAT_00ffe06e = 0xffff;
  DAT_00ffe070 = 0x1e;
  DAT_00ffe072 = 0;
  if (_DAT_00ffe0e2 != 0) {
    if (DAT_00ffe06c < 0) {
      DAT_00ffe06c = DAT_00ffe06c + -8;
    }
    else {
      DAT_00ffe06c = DAT_00ffe06c + 8;
    }
  }
  return;
}


