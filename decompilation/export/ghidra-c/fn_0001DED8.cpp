// Ghidra headless export
// Entry: 0x0001DED8
// Name: FUN_0001ded8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001E514 FUN_0001e514


void FUN_0001ded8(void)

{
  short sVar1;
  undefined *puVar2;
  undefined *extraout_A0;
  
  puVar2 = &DAT_00ff0400;
  sVar1 = 6;
  DAT_00fffc3c = 0;
  do {
    if ((puVar2[0xb0] & 0x80) != 0) {
      FUN_0001e514();
      puVar2 = extraout_A0;
    }
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


