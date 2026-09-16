// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00003D4A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00003d4a(void)

{
  int iVar1;
  int extraout_D1;
  int iVar2;
  short unaff_D3w;
  ushort unaff_D4w;
  ushort uVar3;
  short sVar4;
  int extraout_A0;
  int extraout_A1;
  
  FUN_00003fd8();
  uVar3 = (ushort)-((unaff_D4w & 0x7f) - 0x80) >> 1;
  if ((short)(uVar3 - 1) < 0x2b) {
    FUN_00003d80();
    sVar4 = -(uVar3 - 0x2b);
    FUN_00000e9c();
    iVar1 = (uint)*(ushort *)(extraout_A1 + unaff_D3w) << 5;
    iVar2 = extraout_D1;
    do {
      _DAT_00c00000 = *(ushort *)(extraout_A0 + iVar2 + iVar1);
      if ((short)_DAT_00ffdf58 < 0) {
        _DAT_00c00000 = _DAT_00c00000 ^ 0x8000;
      }
      else {
        _DAT_00c00000 = _DAT_00ffdf58 | _DAT_00c00000;
      }
      uVar3 = (short)iVar2 + 2;
      if ((uVar3 & 7) == 0) {
        uVar3 = (short)iVar2 - 6;
        unaff_D3w = unaff_D3w + 2;
        iVar1 = (uint)*(ushort *)(extraout_A1 + unaff_D3w) << 5;
      }
      iVar2 = CONCAT22((short)((uint)iVar2 >> 0x10),uVar3);
      sVar4 = sVar4 + -1;
    } while (sVar4 != -1);
    return;
  }
  FUN_00003d80();
  return;
}


