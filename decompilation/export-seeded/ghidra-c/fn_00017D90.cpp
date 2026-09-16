// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00017D90


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00017d90(void)

{
  char cVar1;
  char cVar2;
  char in_D1b;
  char *pcVar3;
  
  pcVar3 = (char *)(_DAT_00fffc04 + 0x4c);
  cVar1 = *pcVar3;
  cVar2 = in_D1b + cVar1;
  *pcVar3 = cVar2;
  if (cVar2 == '\0' || SCARRY1(in_D1b,cVar1) != cVar2 < '\0') {
    *pcVar3 = '\0';
  }
  else if ('\n' < *pcVar3) {
    *pcVar3 = '\n';
  }
  FUN_00001232();
  FUN_0001792e();
  return;
}


