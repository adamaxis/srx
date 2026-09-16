// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005D65A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005d65a(void)

{
  short sVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  char unaff_D5b;
  int in_A0;
  short *psVar5;
  ushort *puVar6;
  short *unaff_A3;
  
  sVar4 = *(short *)(in_A0 + 0x20) + -1;
  *(short *)(in_A0 + 0x20) = sVar4;
  if (sVar4 < 0) {
    *(undefined2 *)(in_A0 + 0x20) = 4;
    *(short *)(in_A0 + 0x1e) = *(short *)(in_A0 + 0x1e) + 1;
    if (0x27 < *(short *)(in_A0 + 0x1e)) {
      *(undefined2 *)(in_A0 + 0x1e) = 0;
    }
  }
  sVar1 = *(short *)(in_A0 + 0x18) * 4;
  sVar2 = *(short *)(sVar1 + 0x1fe926);
  sVar4 = *(short *)(in_A0 + 10);
  sVar3 = *(short *)(sVar1 + 0x1fe928);
  sVar1 = *(short *)(in_A0 + 0xc);
  psVar5 = *(short **)((short)(*(short *)(in_A0 + 0x1e) * 4) + 0x1fe946);
  while( true ) {
    if (*psVar5 == -0x8000) break;
    unaff_A3[3] = sVar4 + sVar2 + 0x12 + *psVar5;
    *unaff_A3 = sVar1 + sVar3 + 0x30 + psVar5[1];
    puVar6 = (ushort *)(psVar5 + 3);
    *(char *)(unaff_A3 + 1) = (char)psVar5[2];
    *(char *)((int)unaff_A3 + 3) = unaff_D5b;
    psVar5 = psVar5 + 4;
    unaff_A3[2] = *puVar6 | 0x4000;
    unaff_D5b = unaff_D5b + '\x01';
    _DAT_00ffdd02 = _DAT_00ffdd02 + 1;
    unaff_A3 = unaff_A3 + 4;
    _DAT_00ffdd18 = unaff_A3;
    _DAT_00ffdd1c = _DAT_00ffdd02;
  }
  return;
}


