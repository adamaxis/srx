// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00019432


void FUN_00019432(void)

{
  ushort uVar1;
  
  if (((DAT_00ffdd22 & 4) != 0) || ((DAT_00ffdd22 & 8) != 0)) {
    uVar1 = (ushort)DAT_00fff0d4;
    if ((((&DAT_00fffbd8)[(short)uVar1] != '\x03') &&
        ((((&DAT_00fffbd8)[(short)uVar1] != '\x05' && ((&DAT_00fffbd8)[(short)uVar1] != '\x06')) &&
         ((&DAT_00fffbd8)[(short)uVar1] != '\a')))) &&
       (-1 < (char)(*(char *)((short)uVar1 + 0xfff0e8) + -6))) {
      FUN_00017c84();
    }
    FUN_000194ac();
    FUN_00017bc0();
  }
  return;
}


