// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00013C74


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00013c74(void)

{
  char cVar1;
  short sVar2;
  short *unaff_A2;
  short *psVar3;
  char *pcVar4;
  char *pcVar5;
  
  psVar3 = unaff_A2 + 1;
  sVar2 = *unaff_A2;
  pcVar4 = &DAT_00ffdd2a;
  _DAT_00ffdd28 = 0;
  do {
    cVar1 = *(char *)psVar3;
    if ((((cVar1 == '\0') || (cVar1 == '\x01')) || (cVar1 == '\a')) || (cVar1 == '\b')) {
      _DAT_00ffdd28 = _DAT_00ffdd28 + 1;
      pcVar5 = pcVar4 + 1;
      *pcVar4 = cVar1;
      pcVar4 = pcVar4 + 2;
      *pcVar5 = *(char *)((int)psVar3 + 1);
    }
    else if (cVar1 == '\v') {
      DAT_00ffdd66 = *(char *)((int)psVar3 + 1);
    }
    psVar3 = psVar3 + 1;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  pcVar4[0] = -1;
  pcVar4[1] = -1;
  return;
}


