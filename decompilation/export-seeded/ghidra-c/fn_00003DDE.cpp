// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00003DDE


/* WARNING: Possible PIC construction at 0x00003e04: Changing call to branch */
/* WARNING: Removing unreachable block (ram,0x00003e08) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00003dde(void)

{
  short sVar1;
  ushort uVar2;
  int iVar3;
  int extraout_D1;
  int iVar4;
  short unaff_D3w;
  ushort unaff_D4w;
  ushort uVar5;
  int extraout_A0;
  int extraout_A1;
  undefined2 uStack_6;
  undefined2 uStack_4;
  ushort uStack_2;
  
  uStack_4 = 0;
  uStack_2 = 0x3de2;
  FUN_00003fd8();
  uVar2 = ((ushort)-((unaff_D4w & 0xf80) - 0x1000) >> 7) - 1;
  if ((short)uVar2 < 0x1f) {
    register0x0000003c = (BADSPACEBASE *)&uStack_6;
    uStack_6 = 0;
    uStack_4 = 0x3e08;
    uVar5 = uVar2;
    uStack_2 = uVar2;
  }
  else {
    uVar5 = 0x1f;
  }
  *(ushort *)((int)register0x0000003c + -2) =
       (ushort)(byte)((uVar2 < 0x1f) << 4 | (0x7fff < uVar2) << 3 | (uVar2 == 0) << 2);
  _DAT_00c00004 = 0x8f80;
  *(undefined4 *)((int)register0x0000003c + -6) = 0x3e2a;
  FUN_00000e9c();
  iVar3 = (uint)*(ushort *)(extraout_A1 + unaff_D3w) << 5;
  iVar4 = extraout_D1;
  do {
    _DAT_00c00000 = *(ushort *)(extraout_A0 + iVar4 + iVar3);
    if ((short)_DAT_00ffdf58 < 0) {
      _DAT_00c00000 = _DAT_00c00000 ^ 0x8000;
    }
    else {
      _DAT_00c00000 = _DAT_00ffdf58 | _DAT_00c00000;
    }
    sVar1 = (short)iVar4 + 8;
    if (0x1e < sVar1) {
      sVar1 = (short)iVar4 + -0x18;
      unaff_D3w = _DAT_00ffddea + unaff_D3w;
      iVar3 = (uint)*(ushort *)(extraout_A1 + unaff_D3w) << 5;
    }
    iVar4 = CONCAT22((short)((uint)iVar4 >> 0x10),sVar1);
    uVar5 = uVar5 - 1;
  } while (uVar5 != 0xffff);
  _DAT_00c00004 = 0x8f02;
  return;
}


