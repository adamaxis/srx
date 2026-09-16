// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000181F8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000181f8(void)

{
  undefined4 uVar1;
  undefined1 uVar2;
  undefined4 uVar3;
  short sVar5;
  uint uVar4;
  ushort uVar6;
  
  _DAT_00c00004 = 0x9100;
  FUN_00001358();
  DAT_00ffef14 = DAT_00ffef14 | 1;
  FUN_000177cc();
  uVar2 = DAT_00fff0b7;
  uVar1 = _DAT_00ffdd08;
  DAT_00fffbca = 10;
  DAT_00fff0b1 = 0;
  _DAT_00ffdd08 = 0;
  FUN_00000f1c();
  FUN_00000f3c();
  _DAT_00c00004 = 0x8134;
  FUN_00000ffe();
  FUN_00000e34();
  FUN_00001530();
  FUN_00017f80();
  FUN_00001728();
  FUN_00017708();
  _DAT_00ffdd02 = 1;
  FUN_00001378();
  _DAT_00ffd980 = 1;
  DAT_00ffef14 = DAT_00ffef14 | 2;
  _DAT_00fff0d0 = 0;
  FUN_00019504();
  FUN_000187aa();
  DAT_00fff10a = 0;
  DAT_00fff109 = 0;
  DAT_00fff0d1 = 3;
  FUN_000189d8();
  _DAT_00c00004 = 0x8174;
  DAT_00fff0d1 = 0;
  FUN_0001882e();
  DAT_00fff10c = 0xff;
LAB_00018326:
  do {
    while( true ) {
      FUN_00000bd2();
      FUN_00000bf2();
      FUN_0000195c();
      uVar3 = FUN_000196f2();
      if (((DAT_00ffdd22 & 1) == 0) && ((DAT_00ffdd22 & 2) == 0)) break;
      uVar3 = FUN_000189d8();
      DAT_00fff0d1 = (char)uVar3 + DAT_00fff0d1;
      if ((short)((ushort)DAT_00fff0d1 << 8) < 0) {
        DAT_00fff0d1 = 3;
      }
      else if (-1 < (char)(DAT_00fff0d1 - 4)) {
        DAT_00fff0d1 = 0;
      }
      _DAT_00ffd986 = (short)CONCAT31((int3)((uint)uVar3 >> 8),DAT_00fff0d1) * 0x28 + 0x94;
      uVar4 = (uint)_DAT_00ffd986;
LAB_000183e0:
      _DAT_00ffd98c = (short)CONCAT31((int3)(uVar4 >> 8),DAT_00fff0d0) * 0x20 + 0x8c;
      DAT_00ffef14 = DAT_00ffef14 | 2;
      FUN_0001882e();
    }
    if ((DAT_00ffdd22 & 4) != 0) {
      uVar4 = 0xffff;
LAB_000183b6:
      DAT_00fff0d0 = (char)uVar4 + DAT_00fff0d0;
      if ((short)((ushort)DAT_00fff0d0 << 8) < 0) {
        DAT_00fff0d0 = 3;
      }
      else if (-1 < (char)(DAT_00fff0d0 - 4)) {
        DAT_00fff0d0 = 0;
      }
      goto LAB_000183e0;
    }
    if ((DAT_00ffdd22 & 8) != 0) {
      uVar4 = CONCAT31((int3)((uint)uVar3 >> 8),1);
      goto LAB_000183b6;
    }
    if ((DAT_00ffdd22 & 0x40) == 0) {
      if ((DAT_00ffdd22 & 0x10) == 0) {
        if ((DAT_00ffdd22 & 0x20) == 0) {
          if ((DAT_00ffdd22 & 0x80) != 0) {
            uVar3 = 0xffff;
            goto LAB_00018670;
          }
          goto LAB_00018326;
        }
        DAT_00fff0b7 = 1;
      }
      else {
        DAT_00fff0b7 = 0;
      }
    }
    else {
      DAT_00fff0b7 = 0xff;
    }
    if (DAT_00fff0d1 == 3) {
      if (DAT_00fff0d0 == 3) {
        if (DAT_00fff0b0 != '\0') {
          FUN_0001897c();
          sVar5 = FUN_00019b40();
          if ((sVar5 == 8) && (DAT_00ffdf84 != '\0')) {
                    /* WARNING: Subroutine does not return */
            FUN_0000c0da();
          }
                    /* WARNING: Subroutine does not return */
          FUN_0000c0da();
        }
      }
      else if (((DAT_00fff0d0 != 2) || (DAT_00fff0b0 != '\0')) &&
              ((**(code **)((int)&DAT_000e2dfe + (int)(short)((ushort)DAT_00fff0d0 << 2)))(),
              DAT_00fff0ad != '\0')) {
        DAT_00fff0ad = '\0';
        uVar3 = 0;
LAB_00018670:
        _DAT_00c00004 = 0x8134;
        FUN_00000e34(uVar3);
        FUN_00000ffe();
        FUN_0000102a();
        DAT_00fff10c = 0;
        _DAT_00ffdd08 = uVar1;
        FUN_00000f1c();
        FUN_00000f3c();
        FUN_000176ea();
        uVar4 = 0;
        DAT_00fff108 = '\0';
        sVar5 = 4;
        do {
          uVar6 = (short)CONCAT31((int3)(uVar4 >> 8),DAT_00fff108) << 1;
          uVar4 = (uint)uVar6;
          if (-1 < *(short *)(&DAT_00fff0f2 + (short)uVar6)) {
            _DAT_00c00004 = 0x8174;
            _DAT_00ffdd48 = *(undefined2 *)(&DAT_00fff0f2 + (short)uVar6);
            DAT_00fff0b7 = uVar2;
            return;
          }
          DAT_00fff108 = DAT_00fff108 + '\x01';
          sVar5 = sVar5 + -1;
        } while (sVar5 != -1);
        DAT_00fff0b7 = uVar2;
        _DAT_00c00004 = 0x8174;
        return;
      }
    }
    else {
      FUN_00019238();
    }
  } while( true );
}


