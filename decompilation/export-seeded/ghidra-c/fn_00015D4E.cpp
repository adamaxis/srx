// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00015D4E


uint FUN_00015d4e(void)

{
  short sVar1;
  ushort unaff_D2w;
  ushort unaff_D3w;
  ushort uVar2;
  ushort uVar3;
  bool bVar4;
  
  if ((short)unaff_D2w < 0) {
    unaff_D2w = -unaff_D2w;
  }
  if ((short)unaff_D3w < 0) {
    unaff_D3w = -unaff_D3w;
  }
  bVar4 = unaff_D3w < unaff_D2w;
  uVar3 = unaff_D3w;
  uVar2 = unaff_D2w;
  if (!bVar4) {
    uVar3 = unaff_D2w;
    uVar2 = unaff_D3w;
  }
  sVar1 = uVar2 + (uVar3 >> 1);
  for (; 0x1f < uVar2; uVar2 = uVar2 >> 1) {
  }
  return (uint)CONCAT12(bVar4 << 4 | ((short)(unaff_D3w - unaff_D2w) < 0) << 3 |
                        (unaff_D3w == unaff_D2w) << 2 | SBORROW2(unaff_D3w,unaff_D2w) << 1 | bVar4,
                        sVar1);
}


