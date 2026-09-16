// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00019F78


void FUN_00019f78(void)

{
  short extraout_D1w;
  short sVar1;
  
  sVar1 = 4;
  do {
    if ((&DAT_00fffbd8)[sVar1] == '\x06') {
      FUN_000195aa();
      *(undefined1 *)(sVar1 + 0xfff0e8) = 0;
      *(short *)(&DAT_00fff0f2 + (short)(sVar1 << 1)) = extraout_D1w * 10;
      FUN_00017bc0();
      FUN_00019762();
      return;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


