// Ghidra headless export
// Entry: 0x0001E216
// Name: FUN_0001e216
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001E25A FUN_0001e25a
//   0x0001E302 FUN_0001e302
//   0x0001E40C FUN_0001e40c


void FUN_0001e216(void)

{
  short in_D0w;
  short sVar1;
  int *unaff_A6;
  bool bVar2;
  short *psVar3;
  
  if (in_D0w == 0) {
    FUN_0001e302();
  }
  else {
    FUN_0001e40c();
  }
  if (DAT_00ffdd50 != 0) {
    sVar1 = 0;
    while( true ) {
      bVar2 = false;
      if (*unaff_A6 == 0) break;
      psVar3 = (short *)((int)register0x0000003c + -4);
      *psVar3 = sVar1;
      *(undefined4 *)((int)register0x0000003c + -8) = 0x1e24a;
      FUN_0001e25a();
      register0x0000003c = (BADSPACEBASE *)((int)register0x0000003c + -2);
      if (bVar2) {
        return;
      }
      sVar1 = *psVar3 + 1;
      unaff_A6 = unaff_A6 + 1;
    }
  }
  return;
}


