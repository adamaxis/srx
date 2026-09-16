// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00053704


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00053704(void)

{
  undefined4 uVar1;
  short sVar2;
  int extraout_A1;
  int iVar3;
  
  FUN_000532bc();
  FUN_000537fa();
  do {
    while( true ) {
      FUN_00000bd2();
      FUN_00000bf2();
      FUN_0000195c();
      if ((DAT_00ffdd22 & 4) == 0) break;
      if (extraout_A1 + -0xff1101 < 0) {
        iVar3 = 0xff1107;
        sVar2 = 6;
        do {
          if (-1 < *(char *)(iVar3 + 7 + (int)(short)(_DAT_00fff118 * 0x14))) goto LAB_000537c6;
          iVar3 = iVar3 + -1;
          sVar2 = sVar2 + -1;
        } while (sVar2 != -1);
      }
      else {
LAB_000537c6:
        FUN_000537fa();
      }
    }
    if ((((DAT_00ffdd22 & 0x40) != 0) || ((DAT_00ffdd22 & 0x10) != 0)) ||
       ((DAT_00ffdd22 & 0x20) != 0)) {
      _DAT_00fff11a = _DAT_00fff118;
      _DAT_00fff118 = (short)*(char *)(extraout_A1 + 7 + (int)(short)(_DAT_00fff118 * 0x14));
      if (DAT_00fff0a9 == '\0') {
        register0x0000003c = (BADSPACEBASE *)&stack0x00000004;
      }
      *(undefined4 *)((int)register0x0000003c + -4) = 0x537b4;
      FUN_00017708();
      return 0xffffffff;
    }
    if ((DAT_00ffdd22 & 8) != 0) goto LAB_000537c6;
    if (((DAT_00ffdd22 & 0x80) != 0) && (DAT_00fff0a9 == '\0')) {
      _DAT_00c00004 = 0x8134;
      FUN_000177cc();
      uVar1 = FUN_000176ea();
      return uVar1;
    }
  } while( true );
}


