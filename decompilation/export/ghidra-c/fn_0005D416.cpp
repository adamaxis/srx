// Ghidra headless export
// Entry: 0x0005D416
// Name: FUN_0005d416
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000530DE service_audio_if_enabled
//   0x000566D8 FUN_000566d8
//   0x0005D6BC FUN_0005d6bc


void FUN_0005d416(void)

{
  int in_A0;
  
  if (0x32 < *(short *)(in_A0 + 0x24)) {
    return;
  }
  if (*(short *)(in_A0 + 0x24) == 0x14) {
    service_audio_if_enabled();
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
    DAT_00ffdd08 = DAT_00ffdd08 + -8;
    DAT_00ffdd0c = DAT_00ffdd0c + -8;
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
    service_audio_if_enabled();
    DAT_00ffdd08 = DAT_00ffdd08 + 8;
    DAT_00ffdd0c = DAT_00ffdd0c + 8;
  }
LAB_0005d522:
  DAT_00ffef14 = DAT_00ffef14 | 0xd;
  return;
}


