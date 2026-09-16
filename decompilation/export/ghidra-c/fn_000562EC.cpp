// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000562EC


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000562ec(void)

{
  char cVar1;
  short sVar2;
  char extraout_D1b;
  char *extraout_A0;
  
  FUN_00055f44();
  FUN_000562de();
  FUN_00000dba();
  cVar1 = DAT_00fff0a4 - DAT_00fff0dd;
  if (cVar1 != '\0') {
    if (cVar1 != '\0' && DAT_00fff0dd <= DAT_00fff0a4) {
      if ((char)(DAT_00fffbbf - cVar1) < '\0') {
        sVar2 = FUN_00019b40();
        if (sVar2 == 8) {
          FUN_00017d90();
          return;
        }
        func_0x0001a12e();
        if (((extraout_D1b < '\0') && (FUN_00017db8(), DAT_00fffbca == '\0')) &&
           (((sVar2 = FUN_00019b40(), sVar2 == 3 ||
             (((sVar2 == 0xc || (sVar2 == 0xf)) || (sVar2 == 0x17)))) || (sVar2 == 0x18)))) {
          FUN_00019bf6();
          FUN_00000dba();
          if ((DAT_00fff0dd != '\0') &&
             (cVar1 = DAT_00fff0dd - extraout_A0[1], cVar1 != '\0' && extraout_A0[1] <= DAT_00fff0dd
             )) {
            cVar1 = *extraout_A0 - cVar1;
            *extraout_A0 = cVar1;
            if (cVar1 < '\0') {
              *extraout_A0 = '\0';
              extraout_A0[7] = '\0';
            }
            if (extraout_A0[7] != '\0') {
              extraout_A0[7] = extraout_A0[7] + -1;
            }
            FUN_0001b1fc();
          }
          return;
        }
      }
    }
    else {
      _DAT_00ffe07e = 1;
    }
  }
  FUN_0001792e();
  return;
}


