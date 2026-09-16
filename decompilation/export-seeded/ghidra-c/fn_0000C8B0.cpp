// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000C8B0


void FUN_0000c8b0(void)

{
  char *pcVar1;
  char cVar2;
  char unaff_D6b;
  short unaff_D7w;
  
  pcVar1 = (char *)((short)(unaff_D7w + -1) + 0xfffc12);
  cVar2 = unaff_D6b + *pcVar1;
  *pcVar1 = cVar2;
  if (cVar2 < '\0') {
    *(undefined1 *)((short)(unaff_D7w + -1) + 0xfffc12) = 0xff;
  }
  return;
}


