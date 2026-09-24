// Ghidra headless export
// Entry: 0x00005936
// Name: FUN_00005936
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 FUN_00001378


void FUN_00005936(void)

{
  byte bVar1;
  int in_A0;
  
  if (*(short *)(in_A0 + 0xe) != 0xf) {
    bVar1 = *(byte *)(in_A0 + 6) - 1;
    *(byte *)(in_A0 + 6) = bVar1;
    if ((short)((ushort)bVar1 << 8) < 0) {
      *(undefined1 *)(in_A0 + 6) = 3;
      *(char *)(in_A0 + 7) = *(char *)(in_A0 + 7) + '\x01';
      if ('\x03' < *(char *)(in_A0 + 7)) {
        *(undefined1 *)(in_A0 + 7) = 0;
      }
    }
    FUN_00001378();
    return;
  }
  bVar1 = *(byte *)(in_A0 + 6) - 1;
  *(byte *)(in_A0 + 6) = bVar1;
  if ((short)((ushort)bVar1 << 8) < 0) {
    *(undefined1 *)(in_A0 + 6) = 3;
    *(char *)(in_A0 + 7) = *(char *)(in_A0 + 7) + '\x01';
    if ('\x03' < *(char *)(in_A0 + 7)) {
      *(undefined1 *)(in_A0 + 7) = 0;
    }
  }
  FUN_00001378(*(short *)(in_A0 + 10) + -0x2e,*(short *)(in_A0 + 0xc) + -0x20,
               *(undefined4 *)(&DAT_00005a46 + (short)(((short)*(char *)(in_A0 + 7) & 3U) << 2)));
  FUN_00001378();
  return;
}


