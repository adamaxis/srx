// Ghidra headless export
// Entry: 0x00019788
// Name: FUN_00019788
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 upload_hud_tile_block
//   0x0001792E FUN_0001792e
//   0x00017C52 FUN_00017c52
//   0x00017C84 FUN_00017c84
//   0x00017EEE FUN_00017eee


void FUN_00019788(void)

{
  short sVar1;
  char *pcVar2;
  
  DAT_00fff0ac = DAT_00fff0ac - 1;
  if ((short)((ushort)DAT_00fff0ac << 8) < 0) {
    DAT_00fff0ac = 0x28;
    FUN_0001792e();
  }
  FUN_00017eee();
  if (DAT_00fff0b8 != '\0') {
    pcVar2 = &DAT_00fff0ed;
    DAT_00ffdd48 = DAT_00ffdd48 + -1;
    if (DAT_00ffdd48 < 0) {
      DAT_00fff108 = 0;
      sVar1 = 4;
      do {
        if (*pcVar2 != '\x06') {
          *pcVar2 = *pcVar2 + '\x01';
          upload_hud_tile_block();
          DAT_00ffdd48 = *(undefined2 *)
                          ((int)&DAT_00fff0f2 + (int)(short)((ushort)DAT_00fff108 << 1));
          if (*pcVar2 != '\x06') {
            return;
          }
          FUN_00017c84();
          FUN_00017c52();
          return;
        }
        pcVar2 = pcVar2 + 1;
        DAT_00fff108 = DAT_00fff108 + 1;
        sVar1 = sVar1 + -1;
      } while (sVar1 != -1);
    }
  }
  return;
}


