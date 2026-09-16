// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000533EE


/* WARNING: Possible PIC construction at 0x000536c2: Changing call to branch */
/* WARNING: Possible PIC construction at 0x00053558: Changing call to branch */
/* WARNING: Removing unreachable block (ram,0x000536c6) */
/* WARNING: Removing unreachable block (ram,0x0005355c) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000533ee(void)

{
  short sVar1;
  short sVar2;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int iVar3;
  undefined1 *puVar4;
  undefined1 *extraout_A1;
  undefined1 *extraout_A1_00;
  char *pcVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  
  FUN_00053200();
  _DAT_00fff102 =
       (short)CONCAT31((int3)((uint)(byte)(&DAT_00ff1104)[(short)(_DAT_00fff118 * 0x14)] * 0xe >> 8)
                       ,*(char *)((short)(_DAT_00fff118 * 0x14) + 0xff1103) +
                        (char)((uint)(byte)(&DAT_00ff1104)[(short)(_DAT_00fff118 * 0x14)] * 0xe));
  puVar4 = &DAT_00fffaa4;
  (&DAT_00fffaa4)[_DAT_00fff102] = 0xff;
  sRam00fff104 = _DAT_00fff102;
  uRam00fffbfc = 0;
  iVar3 = extraout_A0;
code_r0x00053434:
  do {
    sVar1 = sRam00fff104 + -0xe;
    if (-1 < sVar1) {
      if ((char)('\v' - *(char *)(iVar3 + sVar1)) < '\0') {
        sVar2 = 6;
        pcVar5 = (char *)0x15fcaf;
        do {
          if (*pcVar5 == *(char *)(iVar3 + sVar1)) {
            sRam00fff104 = sVar1;
            puVar4[sVar1] = 0xff;
            *(ushort *)((int)register0x0000003c + -2) = uRam00fffbfc;
            uRam00fffbfc = 0x800;
            *(undefined4 *)((int)register0x0000003c + -6) = 0x5349e;
            func_0x00053434();
            uRam00fffbfc = *(ushort *)((int)register0x0000003c + -2);
            sRam00fff104 = sRam00fff104 + 0xe;
            iVar3 = extraout_A0_00;
            puVar4 = extraout_A1;
            break;
          }
          sVar2 = sVar2 + -1;
          pcVar5 = pcVar5 + 1;
        } while (sVar2 != -1);
      }
      else {
        sVar2 = 6;
        pcVar5 = (char *)0x15fca8;
        do {
          if (*pcVar5 == *(char *)(iVar3 + sRam00fff104)) {
            sRam00fff104 = sVar1;
            puVar4[sVar1] = 0xff;
            sRam00fff104 = sRam00fff104 + 0xe;
            break;
          }
          sVar2 = sVar2 + -1;
          pcVar5 = pcVar5 + 1;
        } while (sVar2 != -1);
      }
    }
    if (((uRam00fffbfc & 0x400) == 0) && (sVar1 = sRam00fff104 + 1, -1 < (short)(0x8b - sVar1))) {
      if ((char)('\v' - *(char *)(iVar3 + sVar1)) < '\0') {
        sVar2 = 6;
        pcVar5 = (char *)0x15fc9a;
        do {
          if (*pcVar5 == *(char *)(iVar3 + sVar1)) {
            sRam00fff104 = sVar1;
            puVar4[sVar1] = 0xff;
            *(ushort *)((int)register0x0000003c + -2) = uRam00fffbfc;
            uRam00fffbfc = 0x1000;
            puVar6 = (undefined4 *)((int)register0x0000003c + -6);
            register0x0000003c = (BADSPACEBASE *)((int)register0x0000003c + -6);
            *puVar6 = 0x5355c;
            goto code_r0x00053434;
          }
          sVar2 = sVar2 + -1;
          pcVar5 = pcVar5 + 1;
        } while (sVar2 != -1);
      }
      else {
        sVar2 = 6;
        pcVar5 = (char *)0x15fca1;
        do {
          if (*pcVar5 == *(char *)(iVar3 + sRam00fff104)) {
            sRam00fff104 = sVar1;
            puVar4[sVar1] = 0xff;
            sRam00fff104 = sRam00fff104 + -1;
            break;
          }
          sVar2 = sVar2 + -1;
          pcVar5 = pcVar5 + 1;
        } while (sVar2 != -1);
      }
    }
    if (((uRam00fffbfc & 0x800) == 0) && (sVar1 = sRam00fff104 + 0xe, -1 < (short)(0x8b - sVar1))) {
      if ((char)('\v' - *(char *)(iVar3 + sVar1)) < '\0') {
        sVar2 = 6;
        pcVar5 = (char *)0x15fca8;
        do {
          if (*pcVar5 == *(char *)(iVar3 + sVar1)) {
            sRam00fff104 = sVar1;
            puVar4[sVar1] = 0xff;
            *(ushort *)((int)register0x0000003c + -2) = uRam00fffbfc;
            uRam00fffbfc = 0x200;
            *(undefined4 *)((int)register0x0000003c + -6) = 0x53614;
            func_0x00053434();
            uRam00fffbfc = *(ushort *)((int)register0x0000003c + -2);
            sRam00fff104 = sRam00fff104 + -0xe;
            iVar3 = extraout_A0_01;
            puVar4 = extraout_A1_00;
            break;
          }
          sVar2 = sVar2 + -1;
          pcVar5 = pcVar5 + 1;
        } while (sVar2 != -1);
      }
      else {
        sVar2 = 6;
        pcVar5 = (char *)0x15fcaf;
        do {
          if (*pcVar5 == *(char *)(iVar3 + sRam00fff104)) {
            sRam00fff104 = sVar1;
            puVar4[sVar1] = 0xff;
            sRam00fff104 = sRam00fff104 + -0xe;
            break;
          }
          sVar2 = sVar2 + -1;
          pcVar5 = pcVar5 + 1;
        } while (sVar2 != -1);
      }
    }
    if ((uRam00fffbfc & 0x1000) != 0) {
      return;
    }
    sVar1 = sRam00fff104 + -1;
    if (sVar1 < 0) {
      return;
    }
    if (-1 < (char)('\v' - *(char *)(iVar3 + sVar1))) {
      sVar2 = 6;
      pcVar5 = (char *)0x15fc9a;
      do {
        if (*pcVar5 == *(char *)(iVar3 + sRam00fff104)) {
          sRam00fff104 = sVar1;
          puVar4[sVar1] = 0xff;
          sRam00fff104 = sRam00fff104 + 1;
          return;
        }
        sVar2 = sVar2 + -1;
        pcVar5 = pcVar5 + 1;
      } while (sVar2 != -1);
      return;
    }
    sVar2 = 6;
    pcVar5 = (char *)0x15fca1;
    while (*pcVar5 != *(char *)(iVar3 + sVar1)) {
      sVar2 = sVar2 + -1;
      pcVar5 = pcVar5 + 1;
      if (sVar2 == -1) {
        return;
      }
    }
    sRam00fff104 = sVar1;
    puVar4[sVar1] = 0xff;
    *(ushort *)((int)register0x0000003c + -2) = uRam00fffbfc;
    uRam00fffbfc = 0x400;
    puVar7 = (undefined4 *)((int)register0x0000003c + -6);
    register0x0000003c = (BADSPACEBASE *)((int)register0x0000003c + -6);
    *puVar7 = 0x536c6;
  } while( true );
}


