// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0005D416


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005d416(void)

{
  int in_A0;
  
  if (0x32 < *(short *)(in_A0 + 0x24)) {
    return;
  }
  if (*(short *)(in_A0 + 0x24) == 0x14) {
    FUN_000530de();
    FUN_0005d6bc();
    goto LAB_0005d522;
  }
  if (*(short *)(in_A0 + 0x24) == 0x16) {
LAB_0005d516:
    FUN_000566d8();
    goto LAB_0005d522;
  }
  if (*(short *)(in_A0 + 0x24) == 0x18) {
LAB_0005d506:
    _DAT_00ffdd08 = _DAT_00ffdd08 + -8;
    _DAT_00ffdd0c = _DAT_00ffdd0c + -8;
  }
  else {
    if (*(short *)(in_A0 + 0x24) != 0x19) {
      if (*(short *)(in_A0 + 0x24) == 0x1a) goto LAB_0005d506;
      if (*(short *)(in_A0 + 0x24) != 0x1b) {
        if (*(short *)(in_A0 + 0x24) == 0x1d) goto LAB_0005d506;
        if (*(short *)(in_A0 + 0x24) != 0x1e) {
          if (((*(short *)(in_A0 + 0x24) == 0x20) || (*(short *)(in_A0 + 0x24) == 0x21)) ||
             (*(short *)(in_A0 + 0x24) == 0x23)) goto LAB_0005d516;
          if (*(short *)(in_A0 + 0x24) == 0x24) goto LAB_0005d506;
          if ((*(short *)(in_A0 + 0x24) != 0x25) && (*(short *)(in_A0 + 0x24) != 0x26)) {
            if (*(short *)(in_A0 + 0x24) == 0x27) goto LAB_0005d506;
            if ((*(short *)(in_A0 + 0x24) != 0x28) && (*(short *)(in_A0 + 0x24) != 0x29)) {
              FUN_000566d8();
              goto LAB_0005d522;
            }
          }
        }
      }
    }
    FUN_000530de();
    _DAT_00ffdd08 = _DAT_00ffdd08 + 8;
    _DAT_00ffdd0c = _DAT_00ffdd0c + 8;
  }
LAB_0005d522:
  DAT_00ffef14 = DAT_00ffef14 | 0xd;
  return;
}


