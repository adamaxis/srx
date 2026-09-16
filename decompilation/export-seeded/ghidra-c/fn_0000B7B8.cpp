// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000B7B8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000b7b8(void)

{
  ushort uVar1;
  ushort unaff_D4w;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  int extraout_A0_04;
  int extraout_A0_05;
  int extraout_A0_06;
  int extraout_A0_07;
  int extraout_A0_08;
  int extraout_A0_09;
  int extraout_A0_10;
  int extraout_A0_11;
  int extraout_A0_12;
  byte bVar2;
  bool bVar3;
  
  if ((_DAT_00ffdfe8 != 0) ||
     (bVar2 = *(byte *)(in_A0 + 0xb0), *(byte *)(in_A0 + 0xb0) = bVar2 | 0x20, (bVar2 & 0x20) != 0))
  {
    return;
  }
  if (((*(byte *)(in_A0 + 0x10) & 2) != 0) &&
     (((_DAT_00ffdf40 == 2 || (_DAT_00ffddc4 == 0x14)) || (_DAT_00ffdf84 == 0)))) {
    bVar2 = DAT_00ffef10 < '\0';
    if (DAT_00ffef11 < '\0') {
      bVar2 = bVar2 + 1;
    }
    if (DAT_00ffef12 < '\0') {
      bVar2 = bVar2 + 1;
    }
    if (DAT_00ffef13 < '\0') {
      bVar2 = bVar2 + 1;
    }
    if (1 < bVar2) {
      FUN_0000bc36();
      *(ushort *)(extraout_A0 + 0xcc) = unaff_D4w << 0xe | unaff_D4w >> 2;
      FUN_0000423c();
      uVar1 = _DAT_00ffdf64;
      *(ushort *)(extraout_A0_00 + 0xc6) = _DAT_00ffdf64;
      uVar1 = uVar1 & 0x1f;
      if (uVar1 == 0x1f) {
        _DAT_00ffdd2c = 0xdb26;
        FUN_0001e2e0();
        in_A0 = extraout_A0_01;
      }
      else if (uVar1 == 0x1e) {
        _DAT_00ffdd2c = 0x20990;
        FUN_0001e2e0();
        in_A0 = extraout_A0_02;
      }
      else {
        in_A0 = extraout_A0_00;
        if (uVar1 == 0x1d) {
          if (DAT_00ffe16f == '\0') {
            _DAT_00ffdd2c = 0x58d94;
            FUN_0001e2e0();
            in_A0 = extraout_A0_03;
          }
        }
        else if (uVar1 == 0x1c) {
          if (DAT_00ffe16f == '\0') {
            _DAT_00ffdd2c = 0x58db2;
            FUN_0001e2e0();
            in_A0 = extraout_A0_04;
          }
        }
        else if (uVar1 == 0x1b) {
          _DAT_00ffdd2c = 0x53d5a;
          FUN_0001e2e0();
          in_A0 = extraout_A0_05;
        }
        else if (uVar1 == 0x1a) {
          DAT_00ffe178 = 0;
          bVar2 = (byte)*(undefined2 *)(extraout_A0_00 + 0xc6);
          bVar3 = bVar2 < 0xda;
          if ((bVar2 != 0xda) && ((DAT_00fffc1c & 0x80) == 0)) {
            DAT_00ffe178 = 1;
            bVar3 = false;
          }
          FUN_00016760();
          in_A0 = extraout_A0_06;
          if (bVar3) {
            _DAT_00ffdd2c = 0x20c90;
            FUN_0001e2ec();
            in_A0 = extraout_A0_07;
          }
        }
        else if (((_DAT_00ffddc4 == 0x14) || (_DAT_00ffdf40 == 2)) &&
                ((1 < uVar1 && ((uVar1 < 6 && (DAT_00ffe17f == '\0')))))) {
          _DAT_00ffdd2c = 0x20e48;
          DAT_00ffe16e = FUN_0001e2e0();
          _DAT_00ffe11a = _DAT_00ffdfe4;
          in_A0 = extraout_A0_08;
        }
        else if (uVar1 == 0x18) {
          _DAT_00ffdd2c = 0x20ec2;
          FUN_0001e2e0();
          in_A0 = extraout_A0_09;
        }
        else {
          if (uVar1 == 0x19) {
            if (_DAT_00fffc10 == 0x13) {
              _DAT_00ffdd2c = 0x53efa;
              FUN_0001e2e0();
              in_A0 = extraout_A0_10;
              goto LAB_0000baae;
            }
            DAT_00ffef10 = -0x58;
            _DAT_00ffdf64 = 0xa8;
            *(undefined2 *)(extraout_A0_00 + 0xc6) = 0xa8;
            uVar1 = 8;
          }
          if (((DAT_00ff0110 & 0x40) == 0) && ((DAT_00ff0110 & 0x10) == 0)) {
            DAT_00fffbf9 = (undefined1)uVar1;
            bVar3 = false;
            if (((((_DAT_00ffdf40 == 0) || (bVar3 = _DAT_00ffddc4 < 0x14, _DAT_00ffddc4 == 0x14)) ||
                 (bVar3 = _DAT_00ffdf40 < 2, _DAT_00ffdf40 != 2)) || (bVar3 = false, uVar1 != 0)) ||
               ((_DAT_00ffdf84 == 0 && (bVar3 = false, DAT_00ffe16f == '\0')))) {
              FUN_0001651e();
              if (bVar3) {
                _DAT_00ffdf3a = 0xffff;
                _DAT_00ffdf3c = 0x10;
                _DAT_00ffdfe8 = 0xffff;
                return;
              }
              *(ushort *)(extraout_A0_12 + 0xc6) = _DAT_00ffdf64;
              return;
            }
          }
          else {
            FUN_00053126();
            _DAT_00ffdd2c = 0xc64c;
            FUN_0001e2ec();
            in_A0 = extraout_A0_11;
          }
        }
      }
    }
  }
LAB_0000baae:
  *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) & 0xdf;
  *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) | 1;
  return;
}


