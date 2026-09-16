// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00013A92


void FUN_00013a92(void)

{
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  bool bVar1;
  
  if (*(char *)(in_A0 + 0x9b) < '\x02') {
    bVar1 = false;
    if ((*(char *)(in_A0 + 0x56) < '\0') && (FUN_00013bba(), in_A0 = extraout_A0, bVar1)) {
      FUN_00013cd4();
      if (!bVar1) {
        return;
      }
      if (((-1 < *(char *)(extraout_A0_00 + 0x56)) && (*(char *)(extraout_A0_00 + 0x57) != '\0')) &&
         (*(char *)(extraout_A0_00 + 0x57) < '\r')) {
        return;
      }
      FUN_00013ae6();
      return;
    }
    bVar1 = *(byte *)(in_A0 + 0x9c) < 5;
    if ('\x04' < (char)*(byte *)(in_A0 + 0x9c)) {
      FUN_00013b12();
      if (bVar1) {
        FUN_00013af2();
        FUN_00013466();
        return;
      }
      return;
    }
  }
  return;
}


