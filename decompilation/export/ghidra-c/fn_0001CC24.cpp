// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001CC24


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001cc24(void)

{
  byte bVar2;
  ushort uVar1;
  undefined2 extraout_D1w;
  undefined2 unaff_D2w;
  undefined2 uVar3;
  short sVar4;
  int extraout_A0;
  int extraout_A0_00;
  int iVar5;
  short *psVar6;
  undefined4 unaff_A2;
  
  if (DAT_00ff15b8 != '\0') {
    FUN_0001d048();
    FUN_0001d0de();
    bVar2 = *(byte *)(extraout_A0 + 8);
    if ((_DAT_00ffdf88 & 3) == 0) {
      bVar2 = bVar2 + 1;
    }
    if (5 < bVar2) {
      bVar2 = 0;
    }
    *(byte *)(extraout_A0 + 8) = bVar2;
    uVar1 = (short)(char)bVar2 << 3;
    iVar5 = (int)(short)uVar1;
    *(undefined2 *)(iVar5 + 0xffe012) = extraout_D1w;
    *(undefined4 *)(iVar5 + 0xffe014) = unaff_A2;
    *(undefined2 *)(iVar5 + 0xffe018) = unaff_D2w;
    sVar4 = 6;
    uVar3 = 0x6000;
    psVar6 = (short *)(iVar5 + 0xffe01a);
    iVar5 = extraout_A0;
    do {
      if (*psVar6 != 0) {
        *(short *)((int)register0x0000003c + -4) = sVar4;
        *(undefined2 *)((int)register0x0000003c + -8) = uVar3;
        *(ushort *)((int)register0x0000003c + -0xc) = uVar1;
        *(undefined4 *)((int)register0x0000003c + -0x10) = 0x1ccaa;
        FUN_00001378();
        uVar1 = *(ushort *)((int)register0x0000003c + -0xc);
        uVar3 = *(undefined2 *)((int)register0x0000003c + -10);
        sVar4 = *(short *)((int)register0x0000003c + -8);
        register0x0000003c = (BADSPACEBASE *)((int)register0x0000003c + -6);
        iVar5 = extraout_A0_00;
      }
      uVar1 = uVar1 + 8;
      if (0x2f < uVar1) {
        uVar1 = 0;
      }
      psVar6 = (short *)((short)uVar1 + 0xffe012);
      sVar4 = sVar4 + -1;
    } while (sVar4 != -1);
    if (*(char *)(iVar5 + 6) == '\0') {
      _DAT_00ffe0d6 = 0;
      *(undefined4 *)((int)register0x0000003c + -4) = 0x1ccda;
      FUN_00019ea6();
    }
    return;
  }
  return;
}


