// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001494C


void FUN_0001494c(void)

{
  ushort uVar1;
  ushort uVar2;
  ushort unaff_D4w;
  ushort unaff_D5w;
  int unaff_A4;
  
  uVar1 = unaff_D4w & 0x3f;
  uVar2 = unaff_D4w;
  do {
    if ((unaff_D5w & 0x3f) == uVar1) {
      uVar1 = unaff_D5w & 0xffc0;
      do {
        if ((unaff_D4w & 0xffc0) == uVar1) {
          return;
        }
        if ((unaff_D4w & 0xffc0) < uVar1) {
          uVar1 = uVar1 - 0x40;
          unaff_D5w = unaff_D5w - 0x40;
        }
        else {
          uVar1 = uVar1 + 0x40;
          unaff_D5w = unaff_D5w + 0x40;
        }
      } while (*(char *)(unaff_A4 + (short)unaff_D5w) == '\0');
      return;
    }
    if ((unaff_D5w & 0x3f) < uVar1) {
      uVar1 = uVar1 - 1;
      uVar2 = uVar2 - 1;
    }
    else {
      uVar1 = uVar1 + 1;
      uVar2 = uVar2 + 1;
    }
  } while (*(char *)(unaff_A4 + (short)uVar2) == '\0');
  return;
}


