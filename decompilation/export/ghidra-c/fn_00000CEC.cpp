// Ghidra headless export
// Entry: 0x00000CEC
// Name: FUN_00000cec
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00000cec(void)

{
  short sVar1;
  undefined2 *in_A0;
  word *pwVar2;
  
  DAT_00ffdd16 = *in_A0;
  DAT_00ffdd14 = in_A0[1];
  DAT_00ffdd10 = in_A0[2];
  DAT_00ffdd12 = in_A0[3];
  sVar1 = 0xe;
  pwVar2 = in_A0 + 4;
  do {
    VDP_CTRL = *pwVar2;
    sVar1 = sVar1 + -1;
    pwVar2 = pwVar2 + 1;
  } while (sVar1 != -1);
  return;
}


