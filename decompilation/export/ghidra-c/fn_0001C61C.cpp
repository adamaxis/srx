// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001C61C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001c61c(void)

{
  short sVar1;
  short sVar2;
  char unaff_D5b;
  short *psVar3;
  ushort *puVar4;
  short *unaff_A3;
  short sStack_a;
  
  sVar2 = _DAT_00ff1590;
  sVar1 = _DAT_00ff158e;
  DAT_00ff1594 = DAT_00ff1594 + -1;
  if (DAT_00ff1594 < '\0') {
    DAT_00ff1593 = DAT_00ff1593 + '\x01';
    DAT_00ff1594 = (char)*(undefined2 *)
                          ((int)&PTR_DAT_0001c6d6 + (int)(short)((short)DAT_00ff1593 << 2));
    if ('\x1f' < DAT_00ff1593) {
      DAT_00ff1592 = DAT_00ff1592 & 0xf6;
      FUN_0001b612();
      _DAT_00ffe0d6 = 0;
      FUN_00019cb2();
      return;
    }
  }
  psVar3 = *(short **)
            ((int)&PTR_DAT_0001c6bc +
            (int)(short)((short)(char)(&DAT_0001c6d4)[(short)((short)DAT_00ff1593 << 2)] << 2));
  if ((DAT_00ff1592 & 8) != 0) {
    return;
  }
  FUN_00001378();
  while( true ) {
    if (*psVar3 == -0x8000) break;
    unaff_A3[3] = (sVar1 - sVar2) + *psVar3;
    *unaff_A3 = sStack_a + psVar3[1];
    puVar4 = (ushort *)(psVar3 + 3);
    *(char *)(unaff_A3 + 1) = (char)psVar3[2];
    *(char *)((int)unaff_A3 + 3) = unaff_D5b;
    psVar3 = psVar3 + 4;
    unaff_A3[2] = *puVar4 | 0x6000;
    unaff_D5b = unaff_D5b + '\x01';
    _DAT_00ffdd02 = _DAT_00ffdd02 + 1;
    unaff_A3 = unaff_A3 + 4;
    _DAT_00ffdd18 = unaff_A3;
    _DAT_00ffdd1c = _DAT_00ffdd02;
  }
  return;
}


