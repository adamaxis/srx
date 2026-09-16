// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00054662


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_00054662(void)

{
  short sVar1;
  short sVar2;
  
  sVar2 = _DAT_00fff102;
  DAT_00fff0b7 = 0xff;
  DAT_00fff0d4 = 0;
  (&DAT_00fffb30)[_DAT_00fff102] = (&DAT_00fffb30)[_DAT_00fff102] & 0xf;
  (&DAT_00fffb30)[sVar2] = (&DAT_00fffb30)[sVar2] | 0x40;
  sVar1 = _DAT_00fff118 * 0x14;
  (&DAT_00fffb30)[sVar2] = *(byte *)(sVar1 + 0xff1105) | (&DAT_00fffb30)[sVar2];
  if ((&DAT_00ff1100)[sVar1] == '\x01') {
    FUN_000532a8();
  }
  DAT_00fff0a6 = 0xff;
  DAT_00fff0a8 = 0;
  _DAT_00c00004 = 0x8134;
  FUN_00010952();
  FUN_000566f6();
  FUN_00000f5c();
  FUN_00000f5c();
  sVar2 = 5;
  do {
    FUN_00001232();
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  FUN_00001232();
  FUN_000549f6();
  FUN_00017f80();
  _DAT_00c00004 = 0x8174;
  FUN_00054a8e();
  if (DAT_00fff0b1 < '\0') {
    return 0;
  }
                    /* WARNING: Subroutine does not return */
  FUN_0000c0da();
}


