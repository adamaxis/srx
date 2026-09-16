// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000C376


void FUN_0000c376(void)

{
  char cVar1;
  char unaff_D2b;
  char unaff_D3b;
  bool bVar2;
  
  FUN_0000c3b2();
  bVar2 = SCARRY1(unaff_D2b,DAT_00fff0dc);
  cVar1 = unaff_D2b + DAT_00fff0dc;
  DAT_00fff0dc = unaff_D3b;
  if ((bVar2 == cVar1 < '\0') && (DAT_00fff0dc = cVar1, (char)(unaff_D3b - cVar1) < '\0')) {
    DAT_00fff0dc = '\0';
  }
  FUN_0000c3b2();
  DAT_00fff0fc = 0;
  return;
}


