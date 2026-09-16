// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000075E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000075e(void)

{
  char cVar1;
  byte bVar2;
  short *psVar3;
  short *psVar4;
  undefined4 *puVar5;
  short *psVar6;
  short sVar7;
  byte bVar8;
  uint uVar9;
  short sVar10;
  short unaff_D6w;
  short sVar11;
  char *pcVar12;
  byte *pbVar13;
  char *extraout_A1;
  undefined2 *puVar15;
  undefined4 *puVar16;
  undefined1 *puVar18;
  bool bVar19;
  byte *pbVar14;
  undefined4 *puVar17;
  
  FUN_00000c2c();
  FUN_0005d746();
  if (_DAT_00fff9e8 != 0x12345678) {
    _DAT_00fff9e8 = 0x12345678;
  }
  _DAT_00fff9f8 = (short)_DAT_00fff9ec + _DAT_00fff9f8;
  _DAT_00fffc08 = 1;
  _DAT_00fffc0a = 1;
  _DAT_00ffdfe8 = 0xffff;
  FUN_00001a9e();
  FUN_00053126();
  if (_DAT_00ffddc0 != 0) {
    FUN_00059318();
    FUN_00059092();
    FUN_0000131c();
    if (_DAT_00ffe108 == 0) {
      FUN_00059c5e();
    }
LAB_000007f0:
    FUN_00053126();
    FUN_00053290();
    _DAT_00fffbfe = 0x14;
    uRam00fffc12 = 0;
    uRam00fffc16 = 0;
    uRam00fffc1a = 0;
    _DAT_00fffc1e = 0;
    _DAT_00fffc22 = 0;
    DAT_00fffc26 = 0;
    DAT_00fffc27 = 0;
    _DAT_00fffc28 = 0;
    _DAT_00fffc2a = 0;
    _DAT_00fffc2c = 0;
    _DAT_00fffc34 = 0;
    _DAT_00fffc38 = 0;
    _DAT_00fffc30 = 1;
    _DAT_00ffddc4 = 2;
    DAT_00fffbf8 = 2;
    func_0x00003084();
    _DAT_00ffdfe8 = 0xffff;
    DAT_00fff0a0 = 0xff;
    func_0x0000a7b4();
    FUN_0001e68c();
    func_0x000566a2();
    func_0x00026938();
    func_0x0000281e();
    if (_DAT_00ffe108 != 0) {
      func_0x000042b0();
      func_0x000594e0();
      func_0x000028c2();
      _DAT_00ffe108 = 0;
    }
code_r0x000008c0:
    do {
      sVar11 = 0x1e;
      _DAT_00ffdfe8 = 0;
      DAT_00ffe18a = 1;
      _DAT_00ffdd22 = 0;
      _DAT_00ffdd26 = 0;
      _DAT_00ffdd04 = 0xffff;
      uRam00ffdd06 = 0xffff;
      do {
        func_0x0000a8ea();
        func_0x00003a1c();
        func_0x00004618();
        DAT_00ffef14 = DAT_00ffef14 | 0xe;
        sVar11 = sVar11 + -1;
      } while (sVar11 != -1);
      FUN_00000bd2();
      _DAT_00ffdfe8 = 0xffff;
      FUN_000012f6();
      DAT_00ffe18a = 0;
      FUN_0000195c();
      FUN_0000195c();
      _DAT_00ffdfe6 = 0;
      _DAT_00ffdfe8 = 0;
      do {
        do {
          FUN_00000bd2();
          if (sRam00ffdf3e != 0) {
            DAT_00ffe172 = 0xff;
            sRam00ffdf3e = 0;
            FUN_00053126();
            if (DAT_00ffe186 == '\0') {
              bVar19 = false;
              if (_DAT_00ffdf40 != 0) goto code_r0x00000a72;
              func_0x00003168();
              if (!bVar19) {
                func_0x0001f5ca();
                DAT_00ffe18b = '\0';
                if (unaff_D6w != 0) {
                  uRam00ff012e = 0x8000;
                  uRam00ff0134 = 0x8000;
                  _DAT_00ff022e = 0x8000;
                  uRam00ff0234 = 0x8000;
                  uRam00ff032e = 0x8000;
                  uRam00ff0334 = 0x8000;
                  _DAT_00ffdfe8 = 0xffff;
                  func_0x00002fa4();
                  goto code_r0x00000b9c;
                }
                if ((_DAT_00ffddc4 != 4) || (DAT_00fffbf9 != 6)) {
                  if ((_DAT_00fffc0e == 0) || (DAT_00fffbf9 < 8)) goto code_r0x00000a32;
                  func_0x00002ce0();
                }
              }
            }
            else {
              DAT_00ffe186 = '\x02';
code_r0x00000a32:
              func_0x000167ae();
              if (_DAT_00ffddc4 == 0x37) {
                _DAT_00ffdd2c = 0x20938;
                FUN_0001e2ec();
              }
            }
            _DAT_00ffdfe8 = 0xffff;
            _DAT_00ffdf8e = 0xffff;
            goto code_r0x00000b9c;
          }
          if (sRam00ffdf42 != 0) {
code_r0x00000a72:
            if (DAT_00ffe18f != '\0') {
              DAT_00ffe18f = '\0';
              uRam00fffc1a = uRam00fffc1a | 0x8000;
            }
            _DAT_00ffdfe8 = 0xffff;
            func_0x0001e43c();
            func_0x00002f32();
            sRam00ffdf42 = 0;
            goto code_r0x000008c0;
          }
          FUN_00000bf2();
          FUN_0000195c();
          func_0x0000a8ea();
          func_0x00003a1c();
          func_0x00004618();
          func_0x00016396();
          func_0x0001dd94();
          DAT_00ffef14 = DAT_00ffef14 | 0xe;
        } while (((DAT_00ffe18b != '\0') || (_DAT_00ffe166 != 0)) || (_DAT_00ffdfe8 != 0));
        if ((_DAT_00ffdd22 & 0x2000) != 0) {
          FUN_0000391a();
        }
        if ((_DAT_00ffdd22 & 0x8000) != 0) {
          FUN_0005315e();
          _DAT_00ffdfe8 = 0xffff;
          func_0x00010d30();
          FUN_00053156();
          func_0x0001dc22();
          if (_DAT_00ffe108 == 0) goto code_r0x00000b9c;
          func_0x000042b0();
          goto LAB_000007f0;
        }
      } while (_DAT_00ffe104 == (code *)0x0);
      _DAT_00ffdfe8 = 0xffff;
      (*_DAT_00ffe104)();
      if (_DAT_00ffe104 == (code *)0x5bac4) {
        _DAT_00ffe104 = (code *)0x0;
        break;
      }
      _DAT_00ffe104 = (code *)0x0;
code_r0x00000b9c:
      FUN_00002bd2();
    } while( true );
  }
  FUN_0000131c();
  psVar6 = DAT_000002da;
  puVar5 = DAT_000002d6;
  psVar4 = DAT_000002d2;
  psVar3 = DAT_000002ce;
  sVar11 = DAT_000002c8;
  cVar1 = *(char *)((short)(ushort)(DAT_00a10001 >> 6) + 0x46c);
  if (cVar1 != '\0') {
    pcVar12 = &DAT_000001f0;
    sVar7 = 0xf;
    do {
      if (cVar1 == *pcVar12) {
        uVar9 = (uint)DAT_000002c4;
        sVar7 = 0x17;
        puVar17 = (undefined4 *)&DAT_000002de;
        do {
          puVar16 = (undefined4 *)((int)puVar17 + 1);
          sVar10 = (short)CONCAT31((int3)(uVar9 >> 8),*(undefined1 *)puVar17);
          *psVar6 = sVar10;
          uVar9 = (uint)(ushort)(sVar11 + sVar10);
          sVar7 = sVar7 + -1;
          puVar17 = puVar16;
        } while (sVar7 != -1);
        *(undefined4 *)psVar6 = *puVar16;
        *(undefined2 *)puVar5 = 0;
        sVar7 = 0x3ffe;
        do {
          sVar7 = sVar7 + -1;
        } while (sVar7 != -1);
        *psVar3 = sVar11;
        *psVar4 = sVar11;
        *(undefined4 *)psVar6 = DAT_00000320;
        *(undefined4 *)psVar6 = DAT_00000324;
        sVar11 = 0x1f;
        do {
          *puVar5 = 0;
          sVar11 = sVar11 + -1;
        } while (sVar11 != -1);
        *(undefined4 *)psVar6 = DAT_00000328;
        sVar11 = 0x13;
        do {
          *puVar5 = 0;
          sVar11 = sVar11 + -1;
        } while (sVar11 != -1);
        sVar11 = 3;
        puVar18 = &DAT_0000032c;
        do {
          *(undefined1 *)((int)puVar5 + 0x11) = *puVar18;
          sVar11 = sVar11 + -1;
          puVar18 = puVar18 + 1;
        } while (sVar11 != -1);
        FUN_0000073a();
        FUN_0000075e();
        return;
      }
      pcVar12 = pcVar12 + 1;
      sVar7 = sVar7 + -1;
    } while (sVar7 != -1);
    _DAT_00c00004 = 0x40000000;
    pbVar13 = (byte *)(s_PAL_AND_FRENCH_SECAM_MEGA_DRIVE_000004ce + 0x20);
    sVar11 = 0x3a;
    uVar9 = 0x10000000;
    do {
      sVar7 = 7;
      pbVar14 = pbVar13;
      do {
        pbVar13 = pbVar14 + 1;
        bVar8 = *pbVar14;
        _DAT_00c00000 = 0;
        sVar10 = 7;
        do {
          uVar9 = uVar9 << 4 | uVar9 >> 0x1c;
          bVar2 = bVar8 << 7;
          bVar8 = bVar2 | bVar8 >> 1;
          if (bVar2 != 0) {
            _DAT_00c00000 = uVar9 | _DAT_00c00000;
          }
          sVar10 = sVar10 + -1;
        } while (sVar10 != -1);
        sVar7 = sVar7 + -1;
        pbVar14 = pbVar13;
      } while (sVar7 != -1);
      sVar11 = sVar11 + -1;
    } while (sVar11 != -1);
    FUN_00000436();
    for (pcVar12 = &DAT_000001f0; *pcVar12 != ' '; pcVar12 = pcVar12 + 1) {
      for (puVar15 = &DAT_00000470; (char)*puVar15 != '\0'; puVar15 = puVar15 + 3) {
        if ((char)*puVar15 == *pcVar12) {
          if ((pcVar12[1] == ' ') && (pcVar12 != &DAT_000001f0)) {
            FUN_00000436();
          }
          FUN_00000436();
          pcVar12 = extraout_A1;
          break;
        }
      }
    }
    FUN_00000436();
  }
  do {
                    /* WARNING: Do nothing block with infinite loop */
  } while( true );
}


