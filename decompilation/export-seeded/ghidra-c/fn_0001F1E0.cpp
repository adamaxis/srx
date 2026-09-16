// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001F1E0


void FUN_0001f1e0(void)

{
  char cVar1;
  
  cVar1 = DAT_00fffbfb + -8;
  DAT_00fffbfb = DAT_00fffbfb + '\x01';
  if (-1 < cVar1) {
    DAT_00fffbfb = '\b';
  }
  return;
}


