// Ghidra headless export
// Entry: 0x00016FA8
// Name: FUN_00016fa8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00016fa8(void)

{
  int iVar1;
  short in_D0w;
  short in_D1w;
  short sVar2;
  short unaff_D2w;
  short sVar3;
  short sVar4;
  
  iVar1 = DAT_00ffdde2;
  sVar2 = in_D1w + -1;
  sVar4 = in_D0w + -1;
  sVar3 = unaff_D2w;
  do {
    do {
      *(undefined2 *)((int)&DAT_00ff4700 + (int)unaff_D2w) = *(undefined2 *)(iVar1 + unaff_D2w);
      unaff_D2w = unaff_D2w + 2;
      sVar4 = sVar4 + -1;
    } while (sVar4 != -1);
    unaff_D2w = DAT_00ffddea + sVar3;
    sVar2 = sVar2 + -1;
    sVar4 = in_D0w + -1;
    sVar3 = unaff_D2w;
  } while (sVar2 != -1);
  return;
}


