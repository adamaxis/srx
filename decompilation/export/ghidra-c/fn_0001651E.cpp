// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001651E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001651e(void)

{
  int iVar1;
  short in_D0w;
  short sVar2;
  short sVar3;
  short unaff_D4w;
  int in_A0;
  undefined2 *extraout_A0;
  undefined2 *puVar4;
  int unaff_A2;
  
  if (_DAT_00ffdf40 == 0) {
    if ((_DAT_00ffddc4 == 4) && ((*(ushort *)(in_A0 + 0xc6) & 0x1f) == 6)) {
      _DAT_00ffe114 = 0x1667a;
    }
    if ((*(ushort *)(in_A0 + 0xc6) & 0xe0) == 0xc0) {
      FUN_000164f2();
      *extraout_A0 = 0xffff;
      extraout_A0[3] = _DAT_00ffdd16 + unaff_D4w;
      extraout_A0[6] = 3;
      extraout_A0[7] = 3;
      *(undefined **)(extraout_A0 + 4) = &DAT_0001669a;
    }
  }
  else if (in_D0w != 0) {
    if (_DAT_00ffddc4 == 0x14) {
      *(undefined1 *)(unaff_A2 + _DAT_00ffdfe4) = 0;
      sVar2 = FUN_000166d0();
      *(undefined2 *)(&DAT_00ff2700 + sVar2) = DAT_001cd296;
      *(undefined2 *)((int)_DAT_00ffddea + (int)(&DAT_00ff2700 + sVar2)) = DAT_001cd298;
      _DAT_00ffe100 = 1;
    }
    else if (_DAT_00ffdf40 == 2) {
      *(undefined1 *)(unaff_A2 + _DAT_00ffdfe4) = 0;
      if ((*(ushort *)(in_A0 + 0xc6) & 0xe0) == 0xa0) {
        sVar3 = FUN_0001669c();
        iVar1 = (int)_DAT_00ffddea;
        *(undefined2 *)(&DAT_00ff4700 + (sVar3 - iVar1)) = 3;
        sVar2 = _DAT_00ffddea;
        puVar4 = (undefined2 *)((int)_DAT_00ffddea + (int)(&DAT_00ff4700 + (sVar3 - iVar1)));
        *puVar4 = 4;
        *(undefined2 *)((int)sVar2 + (int)puVar4) = 5;
        _DAT_00ffe102 = 1;
      }
      else {
        sVar2 = FUN_000166d0();
        *(undefined2 *)(&DAT_00ff2700 + sVar2) = 6;
        *(undefined2 *)((int)_DAT_00ffddea + (int)(&DAT_00ff2700 + sVar2)) = 7;
        _DAT_00ffe100 = 1;
      }
    }
    return;
  }
  return;
}


