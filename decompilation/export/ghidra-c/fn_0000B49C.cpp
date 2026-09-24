// Ghidra headless export
// Entry: 0x0000B49C
// Name: FUN_0000b49c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000B732 FUN_0000b732


void FUN_0000b49c(void)

{
  short sVar1;
  undefined2 unaff_D5w;
  undefined2 unaff_D6w;
  int in_A0;
  undefined2 *extraout_A0;
  undefined2 *extraout_A0_00;
  undefined2 *extraout_A0_01;
  undefined2 *puVar2;
  
  if (*(short *)(in_A0 + 0x38) == 0) {
    *(undefined2 *)(in_A0 + 0x38) = 1;
  }
  sVar1 = FUN_0000b732();
  puVar2 = extraout_A0;
  if ((sVar1 != 0) && (-1 < sVar1)) {
    extraout_A0[0x1c] = 2;
    sVar1 = FUN_0000b732();
    puVar2 = extraout_A0_00;
    if ((-1 < sVar1) && (sVar1 != 0)) {
      sVar1 = FUN_0000b732();
      puVar2 = extraout_A0_01;
      if ((sVar1 != 0) || (-1 < sVar1)) goto LAB_0000b528;
    }
  }
  *puVar2 = unaff_D5w;
  puVar2[2] = unaff_D6w;
LAB_0000b528:
  *(undefined4 *)(puVar2 + 0x1d) = *(undefined4 *)(puVar2 + 0x6a);
  return;
}


