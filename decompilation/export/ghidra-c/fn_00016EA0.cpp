// Ghidra headless export
// Entry: 0x00016EA0
// Name: FUN_00016ea0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00016ea0(void)

{
  short in_D0w;
  short in_D1w;
  short sVar1;
  short unaff_D2w;
  short sVar2;
  short sVar3;
  
  sVar1 = in_D1w + -1;
  sVar3 = in_D0w + -1;
  sVar2 = unaff_D2w;
  do {
    do {
      *(undefined2 *)((int)&DAT_00ff4700 + (int)unaff_D2w) = 2;
      unaff_D2w = unaff_D2w + 2;
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
    unaff_D2w = DAT_00ffddea + sVar2;
    sVar1 = sVar1 + -1;
    sVar3 = in_D0w + -1;
    sVar2 = unaff_D2w;
  } while (sVar1 != -1);
  return;
}


