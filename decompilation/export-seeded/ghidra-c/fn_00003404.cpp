// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00003404


void FUN_00003404(void)

{
  short sVar1;
  short sVar2;
  short sVar3;
  
  sVar1 = 2;
  do {
    sVar3 = 3;
    do {
      sVar2 = 3;
      do {
        FUN_00000f5c();
        sVar2 = sVar2 + -1;
      } while (sVar2 != -1);
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
    sVar3 = 5;
    do {
      FUN_00000f5c();
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_00003548();
  return;
}


