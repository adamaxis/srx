// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000BC36


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000bc36(void)

{
  short sVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  short sVar5;
  short *in_A0;
  short *psVar6;
  short *psVar7;
  
  if (_DAT_00ffdf40 != 0) {
    return;
  }
  psVar6 = (short *)&DAT_00ffdf44;
  sVar4 = 0;
  sVar1 = *in_A0;
  sVar2 = in_A0[2];
  sVar5 = 2;
  do {
    sVar3 = sVar4 + 2;
    psVar7 = psVar6 + 1;
    *psVar6 = sVar1 + *(short *)(&DAT_0000bd34 + sVar4);
    sVar4 = sVar4 + 4;
    psVar6 = psVar6 + 2;
    *psVar7 = sVar2 + *(short *)(&DAT_0000bd34 + sVar3);
    sVar5 = sVar5 + -1;
  } while (sVar5 != -1);
  _DAT_00ffdf50 = _DAT_00ffddc6;
  _DAT_00ffdf52 = _DAT_00ffddc8;
  return;
}


