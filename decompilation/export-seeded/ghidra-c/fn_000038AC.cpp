// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000038AC


void FUN_000038ac(void)

{
  int in_A0;
  
  if ((*(char *)(in_A0 + 0x4e) != '\0') && (*(char *)(in_A0 + 0x57) != '\0')) {
    if (*(short *)(in_A0 + 0x3e) < 3) {
      FUN_000036fc();
      return;
    }
    *(char *)(in_A0 + 0x4e) = *(char *)(in_A0 + 0x4e) + -1;
  }
  return;
}


