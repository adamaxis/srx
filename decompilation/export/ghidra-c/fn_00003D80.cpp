// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00003D80


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00003d80(void)

{
  ushort uVar1;
  int iVar2;
  int extraout_D1;
  int iVar3;
  short unaff_D3w;
  short unaff_D7w;
  int extraout_A0;
  int extraout_A1;
  
  FUN_00000e9c();
  iVar2 = (uint)*(ushort *)(extraout_A1 + unaff_D3w) << 5;
  iVar3 = extraout_D1;
  do {
    _DAT_00c00000 = *(ushort *)(extraout_A0 + iVar3 + iVar2);
    if ((short)_DAT_00ffdf58 < 0) {
      _DAT_00c00000 = _DAT_00c00000 ^ 0x8000;
    }
    else {
      _DAT_00c00000 = _DAT_00ffdf58 | _DAT_00c00000;
    }
    uVar1 = (short)iVar3 + 2;
    if ((uVar1 & 7) == 0) {
      uVar1 = (short)iVar3 - 6;
      unaff_D3w = unaff_D3w + 2;
      iVar2 = (uint)*(ushort *)(extraout_A1 + unaff_D3w) << 5;
    }
    iVar3 = CONCAT22((short)((uint)iVar3 >> 0x10),uVar1);
    unaff_D7w = unaff_D7w + -1;
  } while (unaff_D7w != -1);
  return;
}


