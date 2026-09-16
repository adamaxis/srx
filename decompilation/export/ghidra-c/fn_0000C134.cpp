// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000C134


undefined4 FUN_0000c134(void)

{
  undefined4 in_D0;
  char cVar1;
  short in_D1w;
  char *in_A1;
  char *unaff_A5;
  
  do {
    cVar1 = *in_A1;
    if (cVar1 < '\0') {
      if ((char)(cVar1 << 1) < '\0') {
        if (-1 < in_D1w) {
          do {
            *unaff_A5 = ' ';
            in_D1w = in_D1w + -1;
            unaff_A5 = unaff_A5 + 1;
          } while (in_D1w != -1);
        }
        return in_D0;
      }
      cVar1 = -0x80;
    }
    *unaff_A5 = cVar1;
    in_A1 = in_A1 + 1;
    unaff_A5 = unaff_A5 + 1;
  } while( true );
}


