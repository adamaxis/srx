// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00013BBA


void FUN_00013bba(void)

{
  byte bVar1;
  short unaff_D3w;
  int in_A0;
  bool bVar2;
  
  bVar1 = *(byte *)(in_A0 + 0x57);
  if ((((bVar1 == 1) || (bVar1 == 2)) || (bVar1 == 8)) || (bVar1 == 9)) {
    FUN_00013c74();
    bVar2 = false;
    if ((unaff_D3w != 0) && (FUN_00013c20(), bVar2)) {
      return;
    }
  }
  else {
    bVar2 = bVar1 < 0xc;
    if (bVar1 != 0xc) {
      return;
    }
    FUN_00013c20();
    if (!bVar2) {
      return;
    }
  }
  return;
}


