// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005D6BC


void FUN_0005d6bc(void)

{
  undefined2 uVar1;
  undefined2 extraout_D1w;
  short sVar2;
  undefined2 *puVar3;
  undefined2 *extraout_A1;
  undefined2 *puVar4;
  int extraout_A1_00;
  undefined2 unaff_A2w;
  undefined1 in_CF;
  bool bVar5;
  
  uVar1 = FUN_00015a8c();
  if (!(bool)in_CF) {
    sVar2 = 0x7f;
    puVar3 = &DAT_0007361a;
    puVar4 = extraout_A1;
    do {
      *puVar4 = *puVar3;
      sVar2 = sVar2 + -1;
      puVar3 = puVar3 + 1;
      puVar4 = puVar4 + 1;
    } while (sVar2 != -1);
    *(undefined1 **)(extraout_A1 + 0x1d) = &LAB_00015390;
    extraout_A1[0x16] = 0x2000;
    *extraout_A1 = 0x1200;
    extraout_A1[2] = 0x2c0;
    extraout_A1[0x1f] = uVar1;
    extraout_A1[7] = 2;
    *(byte *)(extraout_A1 + 8) = *(byte *)(extraout_A1 + 8) | 0x80;
    *(byte *)(extraout_A1 + 8) = *(byte *)(extraout_A1 + 8) | 1;
    extraout_A1[0xb] = 0xc;
    *(undefined1 *)((int)extraout_A1 + 0x41) = 3;
    extraout_A1[0x22] = 0;
    extraout_A1[0x23] = 0xffff;
    bVar5 = false;
    uVar1 = FUN_0001a2cc();
    if (bVar5) {
      *(byte *)(extraout_A1_00 + 0x10) = *(byte *)(extraout_A1_00 + 0x10) & 0xfe;
    }
    else {
      *(undefined2 *)(extraout_A1_00 + 0x30) = uVar1;
      *(undefined2 *)(extraout_A1_00 + 0x20) = extraout_D1w;
      *(undefined2 *)(extraout_A1_00 + 0x26) = unaff_A2w;
      *(undefined2 *)(extraout_A1_00 + 0x38) = 0x2a;
    }
  }
  return;
}


