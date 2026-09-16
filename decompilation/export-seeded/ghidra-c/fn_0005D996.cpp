// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005D996


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005d996(void)

{
  char unaff_D5b;
  short *psVar1;
  ushort *puVar2;
  short *unaff_A3;
  
  if (-1 < _DAT_00ffdd30) {
    return;
  }
  if (_DAT_00ffdd3c != 0) {
    return;
  }
  _DAT_00ffe066 = _DAT_00ffe066 + -1;
  if (_DAT_00ffe066 < 0) {
    _DAT_00ffe066 = 3;
    _DAT_00ffe064 = _DAT_00ffe064 + 1;
    if (5 < _DAT_00ffe064) {
      _DAT_00ffe064 = 0;
    }
  }
  FUN_00001378();
  psVar1 = *(short **)((int)&PTR_DAT_0005da38 + (int)(short)(_DAT_00ffe064 << 2));
  while( true ) {
    if (*psVar1 == -0x8000) break;
    unaff_A3[3] = *psVar1 + 0xd8;
    *unaff_A3 = psVar1[1] + 0x120;
    puVar2 = (ushort *)(psVar1 + 3);
    *(char *)(unaff_A3 + 1) = (char)psVar1[2];
    *(char *)((int)unaff_A3 + 3) = unaff_D5b;
    psVar1 = psVar1 + 4;
    unaff_A3[2] = *puVar2 | 0x2000;
    unaff_D5b = unaff_D5b + '\x01';
    _DAT_00ffdd02 = _DAT_00ffdd02 + 1;
    unaff_A3 = unaff_A3 + 4;
    _DAT_00ffdd18 = unaff_A3;
    _DAT_00ffdd1c = _DAT_00ffdd02;
  }
  return;
}


