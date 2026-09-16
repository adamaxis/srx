// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001E216


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

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
  if (_DAT_00ffdd50 != 0) {
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


