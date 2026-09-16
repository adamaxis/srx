// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00019976


/* WARNING: Removing unreachable block (ram,0x0001a924) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00019976(void)

{
  byte bVar1;
  undefined4 uVar2;
  short sVar3;
  undefined1 *puVar4;
  bool bVar5;
  
  if (((&DAT_00fffbd8)[(short)(ushort)DAT_00fff0d4] != -1) &&
     (-1 < (char)((&DAT_00fff0ed)[(short)(ushort)DAT_00fff0d4] + -6))) {
    bVar5 = false;
    FUN_00017c8e();
    if (!bVar5) {
      bVar5 = false;
      FUN_00017c8e();
      if (!bVar5) {
        bVar5 = false;
        FUN_00017c8e();
        if (!bVar5) {
          bVar5 = false;
          uVar2 = FUN_00017c8e();
          if ((!bVar5) && (DAT_00fff0e4 == '\f')) {
            if ((DAT_00ffdd22 & 0x40) == 0) {
              if ((DAT_00ffdd22 & 0x10) == 0) {
                DAT_00fff0b7 = 1;
              }
              else {
                DAT_00fff0b7 = 0;
              }
            }
            else {
              DAT_00fff0b7 = 0xff;
            }
            (**(code **)(&DAT_000e2e0a + (short)((short)uVar2 << 2)))();
            FUN_00017ee4();
            if (DAT_00fff0b6 == '\0') {
              bVar5 = false;
              uVar2 = FUN_00017c8e();
              if (!bVar5) {
                bVar5 = false;
                FUN_00017c8e();
                if (!bVar5) {
                  bVar5 = false;
                  FUN_00017c8e();
                  if (!bVar5) {
                    FUN_00017c84();
                    FUN_00017c52();
                  }
                }
                goto LAB_00019a8a;
              }
            }
            else {
              DAT_00fff0b6 = '\0';
            }
            bVar1 = *(byte *)((short)(_DAT_00fff118 * 0x14) + 0xff1113);
            if (-1 < (char)(bVar1 - 0x11)) {
              if ((bVar1 & 1) == 0) {
                sVar3 = 5;
                (&DAT_00fffbcc)[(short)uVar2] = 0;
              }
              else {
                sVar3 = 4;
              }
              DAT_00fff0a0 = 1;
              FUN_0001989a(uVar2);
              _DAT_00ffe0e4 = (ushort)DAT_00fff0d4;
              _DAT_00ffe0e0 = sVar3;
              _DAT_00ffe0de = 1;
              _DAT_00ffe0dc = 0;
              _DAT_00ffe0da = 0xc;
              _DAT_00ffe0d8 = *(undefined2 *)((short)((ushort)DAT_00fff0d4 << 1) + 0x1b278);
              _DAT_00ff164a = 1;
              _DAT_00ffe0bc = sVar3;
              if (sVar3 < 0) {
                puVar4 = &DAT_0001aa10;
              }
              else {
                if (sVar3 == 0x45) {
                  _DAT_00ff164a = 0;
                  _DAT_00ffe080 = 1;
                  return;
                }
                if (sVar3 == 6) {
                  if (_DAT_00ffe0b8 == 0) {
                    _DAT_00ffe0b8 = 1;
                    _DAT_00ffe0ba = 0x47c0;
                  }
                }
                else if ((sVar3 == 7) && (_DAT_00ffe0b6 == 0)) {
                  _DAT_00ffe0b6 = 1;
                  _DAT_00ffe0be = 0x4800;
                }
                puVar4 = *(undefined1 **)(&DAT_0001a9ce + (short)(sVar3 << 2));
              }
              DAT_00ff1644 = *puVar4;
              DAT_00ff1645 = puVar4[1];
              _DAT_00ff1646 = *(undefined4 *)(puVar4 + 2);
              _DAT_00ff1642 = 0;
              _DAT_00ff163e = 0x120;
              _DAT_00ff1640 = 0xd0;
              return;
            }
          }
        }
      }
    }
  }
LAB_00019a8a:
  FUN_0001792e();
  return;
}


