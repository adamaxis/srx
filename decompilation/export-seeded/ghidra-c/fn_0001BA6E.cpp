// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001BA6E


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ba6e(void)

{
  short sVar1;
  short sVar2;
  short sVar3;
  char unaff_D5b;
  short *psVar4;
  short *unaff_A3;
  
  _DAT_00ffe0b0 = _DAT_00ffe0b0 + -1;
  if (_DAT_00ffe0b0 < 0) {
    _DAT_00ffe0b0 = 4;
    _DAT_00ffe0ae = _DAT_00ffe0ae + 1;
    if (3 < _DAT_00ffe0ae) {
      _DAT_00ffe0ac = 0;
      return;
    }
  }
  psVar4 = *(short **)((int)&PTR_DAT_0001d970 + (int)(short)(_DAT_00ffe0ae << 2));
  FUN_000061cc();
  sVar3 = _DAT_00ffe04c;
  sVar2 = _DAT_00ffe04a;
  while( true ) {
    sVar1 = psVar4[4];
    if (sVar1 == -0x8000) break;
    unaff_A3[3] = sVar2 + sVar1;
    *unaff_A3 = sVar3 + psVar4[5];
    *(char *)(unaff_A3 + 1) = (char)psVar4[6];
    *(char *)((int)unaff_A3 + 3) = unaff_D5b;
    unaff_A3[2] = psVar4[7] + 0x780U | 0x6000;
    unaff_D5b = unaff_D5b + '\x01';
    _DAT_00ffdd02 = _DAT_00ffdd02 + 1;
    unaff_A3 = unaff_A3 + 4;
    psVar4 = psVar4 + 4;
    _DAT_00ffdd18 = unaff_A3;
    _DAT_00ffdd1c = _DAT_00ffdd02;
  }
  return;
}


