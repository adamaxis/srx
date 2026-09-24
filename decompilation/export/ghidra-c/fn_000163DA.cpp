// Ghidra headless export
// Entry: 0x000163DA
// Name: FUN_000163da
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_000163da(void)

{
  undefined4 in_D1;
  short sVar1;
  short *psVar2;
  
  if ((DAT_00ffdf40 == 2) && (DAT_00ffe170 != '\0')) {
    return;
  }
  psVar2 = &DAT_00ffd740;
  sVar1 = 9;
  do {
    if (*psVar2 != 0) {
      (**(code **)(psVar2 + 4))(in_D1);
    }
    psVar2 = psVar2 + 0xf;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


