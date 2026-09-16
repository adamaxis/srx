// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00026074


void FUN_00026074(void)

{
  short sVar1;
  short sVar2;
  short unaff_D7w;
  short sVar3;
  undefined4 *in_A1;
  undefined4 *puVar4;
  int iVar5;
  short *unaff_A5;
  int unaff_A6;
  
  if (*(short *)(unaff_A6 + 0x7c) < 1) {
    iVar5 = unaff_A6 + 0x82c;
  }
  else {
    iVar5 = unaff_A6 + 0x5ac;
  }
  *(short *)(unaff_A6 + 0x7c) = *(short *)(unaff_A6 + 0x7c) - unaff_D7w;
  sVar3 = unaff_D7w + -1;
  if (-1 < sVar3) {
    do {
      sVar1 = *(short *)(iVar5 + 6);
      sVar2 = *(short *)(unaff_A6 + 0xc) + -1;
      puVar4 = in_A1;
      do {
        *puVar4 = 0;
        puVar4 = (undefined4 *)((int)(short)(sVar1 + 3) + (int)puVar4);
        sVar2 = sVar2 + -1;
      } while (sVar2 != -1);
      in_A1 = (undefined4 *)((int)*unaff_A5 + (int)in_A1);
      sVar3 = sVar3 + -1;
      unaff_A5 = unaff_A5 + 1;
    } while (sVar3 != -1);
  }
  return;
}


