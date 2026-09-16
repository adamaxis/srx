// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00055740


void FUN_00055740(void)

{
  int in_A0;
  int in_A1;
  
  if ((((*(char *)(in_A1 + 0x9b) != '\x16') && ((*(ushort *)(in_A0 + 0x90) & 2) == 0)) &&
      (*(char *)(in_A0 + 0x9a) != '\x04')) &&
     ((*(char *)(in_A0 + 0x9a) != '\x02' && ((*(byte *)(in_A1 + 0x11) & 0x80) != 0)))) {
    return;
  }
  return;
}


