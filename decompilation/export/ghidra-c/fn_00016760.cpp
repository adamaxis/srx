// Ghidra headless export
// Entry: 0x00016760
// Name: FUN_00016760
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000166D0 FUN_000166d0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00016760(void)

{
  short sVar1;
  short *psVar2;
  
  sVar1 = FUN_000166d0();
  psVar2 = (short *)((int)&DAT_00ff2700 + (int)DAT_00ffddea + (int)sVar1);
  if (*psVar2 == 0xb) {
    return;
  }
  (&DAT_00ffe179)[DAT_00fff0c8] = (&DAT_00ffe179)[DAT_00fff0c8] + '\x01';
  *psVar2 = 0xb;
  _DAT_00ffe100 = 1;
  return;
}


