// Ghidra headless export
// Entry: 0x00056DC4
// Name: FUN_00056dc4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D94 FUN_00000d94
//   0x0000C0DA FUN_0000c0da
//   0x0000C216 FUN_0000c216
//   0x0001275C FUN_0001275c
//   0x000575F8 FUN_000575f8
//   0x00057628 FUN_00057628
//   0x0005766A FUN_0005766a
//   0x0005794E FUN_0005794e
//   0x00057B8C FUN_00057b8c
//   0x00057C46 FUN_00057c46
//   0x00057C5E FUN_00057c5e
//   0x00057C66 FUN_00057c66
//   0x00058D7E FUN_00058d7e


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_00056dc4(void)

{
  byte *pbVar1;
  byte bVar2;
  undefined1 *puVar3;
  int iVar4;
  undefined2 extraout_D0u;
  undefined3 uVar5;
  uint uVar6;
  ushort uVar8;
  int iVar7;
  uint uVar9;
  int extraout_D1;
  int extraout_D1_00;
  short sVar10;
  int extraout_A0;
  char *extraout_A0_00;
  byte *extraout_A0_01;
  char *pcVar11;
  short *psVar12;
  int unaff_A3;
  bool bVar13;
  undefined8 uVar14;
  
  puVar3 = DAT_00fff0c2;
  DAT_00fff112 = 0;
  uVar6 = (uint)DAT_00fff0b6;
  if (-1 < DAT_00fff0b7) {
    if (DAT_00fff0b7 != '\0') {
      if (DAT_00fff0b7 != '\x01') {
        if ((char)(DAT_00fff0b8 + -3) < '\0') {
          if ((-1 < (char)(DAT_00fff0d6 + -9)) || (uVar6 = FUN_00057628(), *extraout_A0_00 < '\0'))
          {
            DAT_00fff112 = 0xff;
            return uVar6;
          }
          FUN_00057c46();
          FUN_0000c0da();
          FUN_0000c216();
          FUN_0000c216();
          bVar13 = extraout_D1 - DAT_00fffbfe < 0;
          if (DAT_00fffbfe < extraout_D1) {
            FUN_0000c0da();
            uVar6 = FUN_00000d94();
            return uVar6;
          }
          FUN_000575f8();
          if (bVar13) {
            FUN_0000c0da();
            uVar6 = FUN_00000d94();
            return uVar6;
          }
          uVar6 = FUN_00057b8c();
          if (DAT_00fff0dc == '\0') {
            DAT_00fffbfe = DAT_00fffbfe - _DAT_00fff0be;
            FUN_00057628();
            bVar2 = *extraout_A0_01;
            uVar6 = FUN_00058d7e();
            *(char *)(unaff_A3 + 0xe + (int)(short)(ushort)bVar2) = DAT_00fff0d6;
          }
          return uVar6;
        }
        if (DAT_00fff0b8 != '\x03') {
          if (DAT_00fff0b8 != '\x05') {
            uVar6 = FUN_0005766a();
            return uVar6;
          }
          if (-1 < (char)(DAT_00fff0d0 - 5)) {
            DAT_00fff112 = 0xff;
            return uVar6;
          }
          if (-1 < DAT_00fff0b3) {
            FUN_00057c46();
            FUN_0000c0da();
            FUN_0000c216();
            FUN_0000c216();
            if (DAT_00fffbfe < extraout_D1_00) {
              FUN_0000c0da();
              uVar6 = FUN_00000d94();
              return uVar6;
            }
            uVar14 = FUN_00057b8c();
            uVar6 = (uint)((ulonglong)uVar14 >> 0x20);
            if (DAT_00fff0dc == '\0') {
              DAT_00fffbfe = DAT_00fffbfe - (int)uVar14;
              pcVar11 = &DAT_00fffbbe;
              uVar6 = (uint)DAT_00fff0d0;
              if (DAT_00fff0d0 != 0) {
                uVar6 = (uint)(ushort)(DAT_00fff0d0 + 7);
                pcVar11 = &DAT_00fffbbe + uVar6;
              }
              *pcVar11 = DAT_00fff0d6;
            }
          }
          DAT_00fff0b3 = 0;
          return uVar6;
        }
        FUN_00057c46();
        uVar8 = DAT_00fff0b6 - 6;
        if (uVar8 != 0) {
          uVar8 = 1;
        }
        sVar10 = (short)((uint)DAT_00fff0d0 * 0xe);
        if ((char)(&DAT_001ab196)[(int)sVar10 + (uint)uVar8 * 0x70] < '\0') {
          DAT_00fff112 = 0xff;
          return (uint)DAT_00fff0d0 * 0xe;
        }
        if (-1 < (char)(DAT_00fffbcb - (&DAT_001ab196)[(int)sVar10 + (uint)uVar8 * 0x70])) {
          FUN_0000c0da();
          uVar6 = FUN_00000d94();
          return uVar6;
        }
        if (DAT_00fffbfe < _DAT_00fff0be) {
          FUN_0000c0da();
          uVar6 = FUN_00000d94();
          return uVar6;
        }
        FUN_0000c0da();
        FUN_0000c216();
        uVar14 = FUN_00057b8c();
        if (DAT_00fff0dc == '\0') {
          DAT_00fffbfe = DAT_00fffbfe - (int)uVar14;
          sVar10 = (short)((ulonglong)uVar14 >> 0x20);
          DAT_00fffbbe = *(undefined1 *)(extraout_A0 + 6 + (int)sVar10);
          DAT_00fffbbf = *(undefined1 *)(extraout_A0 + 7 + (int)sVar10);
          if (-1 < (short)(*(short *)(extraout_A0 + 8 + (int)sVar10) - DAT_00fffbc0)) {
            DAT_00fffbc0 = *(short *)(extraout_A0 + 8 + (int)sVar10);
          }
          if (-1 < (short)(*(short *)(extraout_A0 + 10 + (int)sVar10) - DAT_00fffbc2)) {
            DAT_00fffbc2 = *(short *)(extraout_A0 + 10 + (int)sVar10);
          }
          DAT_00fffbc4 = *(undefined1 *)(extraout_A0 + 0xc + (int)sVar10);
          DAT_00fffbc5 = *(undefined1 *)(extraout_A0 + 0xd + (int)sVar10);
          DAT_00fffbcb = *(char *)(extraout_A0 + sVar10);
        }
        return (uint)((ulonglong)uVar14 >> 0x20);
      }
      FUN_00057c46();
      _DAT_00ffef96 = 0;
      uVar6 = CONCAT22(extraout_D0u,*(ushort *)(DAT_00fffc04 + 0x90));
      DAT_00fff0d5 = DAT_00fff0d0;
      if ((DAT_00fff0d0 != 2) || ((*(ushort *)(DAT_00fffc04 + 0x90) & 8) == 0)) {
        _DAT_00ffef96 = (ushort)(DAT_00fff0d0 == 3);
        if (-1 < (char)(DAT_00fff0d0 - 5)) {
          if (DAT_00fff0d0 == 5) {
            sVar10 = 3;
            uVar9 = 5;
            do {
              if ((uVar6 & 1 << (uVar9 & 0x1f)) != 0) {
                sVar10 = (short)uVar9 + -4;
                if (sVar10 == 4) goto LAB_000572f8;
                DAT_00fff0d5 = (char)sVar10 + 5;
                _DAT_00ffef96 = 1;
                break;
              }
              uVar9 = (uint)(ushort)((short)uVar9 + 1);
              sVar10 = sVar10 + -1;
            } while (sVar10 != -1);
          }
          else if (DAT_00fff0d0 == 6) {
            DAT_00fff0d5 = 9;
            sVar10 = 2;
            uVar9 = 9;
            do {
              if ((uVar6 & 1 << (uVar9 & 0x1f)) != 0) {
                sVar10 = (short)uVar9 + -8;
                if (sVar10 == 3) goto LAB_000572f8;
                DAT_00fff0d5 = (char)sVar10 + 9;
                _DAT_00ffef96 = 1;
                break;
              }
              uVar9 = (uint)(ushort)((short)uVar9 + 1);
              sVar10 = sVar10 + -1;
            } while (sVar10 != -1);
          }
          else {
            DAT_00fff0d5 = DAT_00fff0d0 + 5;
            sVar10 = 2;
            uVar9 = 0xc;
            do {
              if ((uVar6 & 1 << (uVar9 & 0x1f)) != 0) {
                sVar10 = (short)uVar9 + -0xb;
                if (sVar10 == 3) goto LAB_000572f8;
                DAT_00fff0d5 = (char)sVar10 + DAT_00fff0d5;
                _DAT_00ffef96 = 1;
                break;
              }
              uVar9 = (uint)(ushort)((short)uVar9 + 1);
              sVar10 = sVar10 + -1;
            } while (sVar10 != -1);
          }
        }
        if (((uVar6 & 1 << (DAT_00fff0d5 & 0x1f)) == 0) && (_DAT_00fff0be <= DAT_00fffbfe)) {
          FUN_0000c0da();
          FUN_0000c216();
          uVar14 = FUN_00057b8c();
          iVar4 = DAT_00fffc04;
          bVar2 = DAT_00fff0d5;
          uVar6 = (uint)((ulonglong)uVar14 >> 0x20);
          if (DAT_00fff0dc == '\0') {
            iVar7 = (uint)*(byte *)(DAT_00fffc04 + 0x7e) * 10;
            uVar8 = (ushort)DAT_00fff0d5;
            uVar6 = CONCAT31((int3)((uint)iVar7 >> 8),
                             (*(char *)(DAT_00fffc04 + 0x8e) + (char)iVar7) -
                             (&DAT_001aa8a0)[(short)uVar8]);
            if ((DAT_00fff0d5 == 3) && ((*(ushort *)(DAT_00fffc04 + 0x90) & 4) != 0)) {
              uVar6 = (uint)(ushort)((short)uVar6 + 1);
            }
            if ((char)uVar6 < '\x01') {
              FUN_00057c46();
              goto LAB_000572f8;
            }
            *(char *)(DAT_00fffc04 + 0x7e) = (char)(uVar6 / 10);
            *(char *)(iVar4 + 0x8e) = (char)(uVar6 % 10);
            uVar6 = (uint)*(ushort *)(iVar4 + 0x90) | 1 << (bVar2 & 0x1f);
            if (_DAT_00ffef96 != 0) {
              uVar6 = uVar6 & ~(1 << ((ushort)(uVar8 - 1) & 0x1f));
            }
            *(short *)(iVar4 + 0x90) = (short)uVar6;
            DAT_00fffbfe = DAT_00fffbfe - (int)uVar14;
            if ((char)(*(char *)(iVar4 + 0x9b) + -2) < '\0') {
              *(undefined1 *)(iVar4 + 0x7f) = *(undefined1 *)(iVar4 + 0x7e);
              sVar10 = *DAT_00fff10e;
              psVar12 = DAT_00fff10e;
              do {
                if (*(char *)(iVar4 + 0x7e) < *(char *)((int)psVar12 + 3)) {
                  *(undefined1 *)((int)psVar12 + 3) = *(undefined1 *)(iVar4 + 0x7e);
                }
                sVar10 = sVar10 + -1;
                psVar12 = psVar12 + 1;
              } while (sVar10 != -1);
            }
          }
          return uVar6;
        }
      }
LAB_000572f8:
      FUN_0000c0da();
      uVar6 = FUN_00000d94();
      return uVar6;
    }
    if (DAT_00fff0b8 == '\x03') goto LAB_00056f4a;
    if (-1 < (char)(*(char *)(DAT_00fffc04 + 0x9b) + -2)) {
      FUN_00057c46();
      FUN_0000c0da();
      uVar6 = FUN_00000d94();
      return uVar6;
    }
    if ((DAT_00fff0b8 != '\x01') && ((char)(DAT_00fff0b8 + -1) < '\0')) {
      uVar6 = (uint)DAT_00fff0d0 * 4;
      if ((char)(&DAT_001aaf40)[uVar6 + (uint)(ushort)(DAT_00fff0b6 - 3) * 0x20] < '\0') {
        DAT_00fff112 = 0xff;
        return uVar6;
      }
      if (DAT_00fff0c2[1] != '\x06') {
        if (DAT_00fffbfe < _DAT_00fff0be) {
          FUN_00057c46();
          FUN_0000c0da();
          uVar6 = FUN_00000d94();
          return uVar6;
        }
        DAT_00fffbfe = DAT_00fffbfe - _DAT_00fff0be;
        if ((char)DAT_00fff0c2[1] < '\x01') {
          *DAT_00fff10e = *DAT_00fff10e + 1;
        }
        pcVar11 = puVar3 + 1;
        *puVar3 = DAT_00fff0ae;
        *pcVar11 = *pcVar11 + '\x01';
      }
      return uVar6;
    }
LAB_00056e54:
    uVar6 = FUN_0005794e();
    return uVar6;
  }
  if (((DAT_00fff0b8 == '\0') || ((char)(DAT_00fff0b8 + -2) < '\0')) || (DAT_00fff0b8 == '\x02'))
  goto LAB_00056e54;
  if (-1 < (char)(DAT_00fff0b8 + -4)) {
    if (DAT_00fff0b8 == '\x04') {
      FUN_00057c5e();
      bVar2 = *(byte *)(DAT_00fffc04 + 0x66 + (int)(short)(ushort)DAT_00fff0d0);
      if ((char)bVar2 < '\x01') {
        DAT_00fff112 = 0xff;
        return (uint)bVar2;
      }
      if (-1 < (char)(bVar2 - 0xb)) {
        FUN_0000c0da();
        uVar6 = FUN_00000d94();
        return uVar6;
      }
      DAT_00fff0c2 = (undefined1 *)CONCAT13(DAT_00fff0d0,DAT_00fff0c2._1_3_);
      FUN_0000c0da();
      FUN_0000c0da();
      uVar6 = FUN_00000d94();
      return uVar6;
    }
LAB_00056f4a:
    FUN_00057c5e();
    bVar2 = *(byte *)(DAT_00fffc04 + 0x66 + (int)(short)(ushort)DAT_00fff0d0);
    if ((char)bVar2 < '\x01') {
      DAT_00fff112 = 0xff;
      return (uint)bVar2;
    }
    if (((bVar2 != 0x29) && (bVar2 != 0x2a)) && (bVar2 != 0x2b)) {
      FUN_0000c0da();
      FUN_0000c216();
      uVar6 = FUN_00057b8c();
      iVar4 = DAT_00fffc04;
      bVar2 = DAT_00fff0d0;
      if (DAT_00fff0dc == '\0') {
        DAT_00fffbfe = _DAT_00fff0be + DAT_00fffbfe;
        if (*(char *)(DAT_00fffc04 + 0x66 + (int)(short)(ushort)DAT_00fff0d0) ==
            (char)(*(char *)(DAT_00fffc04 + 0x76) + '+')) {
          *(undefined1 *)(DAT_00fffc04 + 0x76) = 0;
        }
        if (bVar2 == *(byte *)(iVar4 + 0x56)) {
          DAT_00fff098 = 0xffff;
        }
        uVar6 = FUN_0001275c();
      }
      return uVar6;
    }
    FUN_0000c0da();
    uVar6 = FUN_00000d94();
    return uVar6;
  }
  FUN_00057c66();
  if (-1 < (char)(DAT_00fff0d0 - 6)) {
    DAT_00fff112 = 0xff;
    return (uint)DAT_00fff0b6 * 0x10;
  }
  uVar5 = (undefined3)((uint)DAT_00fff0b6 * 0x10 >> 8);
  sVar10 = (short)CONCAT31(uVar5,DAT_00fff0c2._0_1_);
  if ((*(byte *)(DAT_00fffc04 + 0x6e + (int)sVar10) & '\x01' << (DAT_00fff0d0 & 7)) != 0)
  goto LAB_0005710e;
  if (DAT_00fff0d0 != 0) {
    sVar10 = (short)CONCAT31(uVar5,DAT_00fff0c2._0_1_);
    if ((char)(*(char *)(DAT_00fffc04 + 0x66 + (int)sVar10) + -8) < '\0') {
      if ((DAT_00fff0d0 != 1) && (DAT_00fff0d0 != 3)) goto LAB_0005710e;
    }
    else if (DAT_00fff0d0 == 1) goto LAB_0005710e;
  }
  if ((DAT_00fff0d0 != 3) || ((*(byte *)(DAT_00fffc04 + 0x6e + (int)sVar10) & 1) == 0)) {
    if (DAT_00fffbfe < _DAT_00fff0be) {
      FUN_0000c0da();
      uVar6 = FUN_00000d94();
      return uVar6;
    }
    FUN_0000c0da();
    FUN_0000c216();
    uVar14 = FUN_00057b8c();
    iVar4 = DAT_00fffc04;
    uVar6 = (uint)((ulonglong)uVar14 >> 0x20);
    if (DAT_00fff0dc == '\0') {
      DAT_00fffbfe = DAT_00fffbfe - (int)uVar14;
      uVar6 = (uint)DAT_00fff0c2._0_1_;
      uVar8 = (ushort)DAT_00fff0c2._0_1_;
      pbVar1 = (byte *)(DAT_00fffc04 + 0x6e + (int)(short)uVar8);
      *pbVar1 = (byte)(1 << (DAT_00fff0d0 & 0x3f)) | *pbVar1;
      if (DAT_00fff0d0 == 0) {
        pbVar1 = (byte *)(iVar4 + 0x6e + (int)(short)uVar8);
        *pbVar1 = *pbVar1 & 0xf7;
      }
    }
    return uVar6;
  }
LAB_0005710e:
  FUN_0000c0da();
  uVar6 = FUN_00000d94();
  return uVar6;
}


