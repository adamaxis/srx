// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005D57E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005d57e(void)

{
  short sVar1;
  short sVar2;
  char unaff_D5b;
  int in_A0;
  short *psVar3;
  ushort *puVar4;
  short *unaff_A3;
  
  sVar1 = *(short *)(in_A0 + 10);
  sVar2 = *(short *)(in_A0 + 0xc);
  psVar3 = *(short **)((short)(*(short *)(in_A0 + 0x18) * 4) + 0x1fea0e);
  while( true ) {
    if (*psVar3 == -0x8000) break;
    unaff_A3[3] = sVar1 + *psVar3;
    *unaff_A3 = sVar2 + psVar3[1];
    puVar4 = (ushort *)(psVar3 + 3);
    *(char *)(unaff_A3 + 1) = (char)psVar3[2];
    *(char *)((int)unaff_A3 + 3) = unaff_D5b;
    psVar3 = psVar3 + 4;
    unaff_A3[2] = *puVar4 | 0x4000;
    unaff_D5b = unaff_D5b + '\x01';
    _DAT_00ffdd02 = _DAT_00ffdd02 + 1;
    unaff_A3 = unaff_A3 + 4;
    _DAT_00ffdd18 = unaff_A3;
    _DAT_00ffdd1c = _DAT_00ffdd02;
  }
  return;
}


