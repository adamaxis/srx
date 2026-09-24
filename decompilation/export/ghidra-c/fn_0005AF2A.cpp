// Ghidra headless export
// Entry: 0x0005AF2A
// Name: advance_title_intro_schedule
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void advance_title_intro_schedule(void)

{
  short *psVar1;
  short *psVar2;
  
  DAT_00ffdd36 = DAT_00ffdd36 + -1;
  if (DAT_00ffdd36 < 0) {
    psVar1 = DAT_00ffdd32 + 1;
    psVar2 = DAT_00ffdd32 + 3;
    DAT_00ffdd32 = psVar2;
    (**(code **)psVar1)();
    DAT_00ffdd36 = *psVar2;
    if (DAT_00ffdd36 == 0) {
      return;
    }
  }
  return;
}


