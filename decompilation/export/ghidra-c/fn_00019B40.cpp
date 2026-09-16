// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00019B40


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_00019b40(void)

{
  byte bVar1;
  uint uVar2;
  short sVar3;
  char cVar4;
  bool bVar5;
  char cVar6;
  
  if (('\0' < *(char *)((short)(_DAT_00fff118 * 0x14) + 0xff110f)) &&
     (bVar1 = *(byte *)((short)(_DAT_00fff118 * 0x14) + 0xff1113), uVar2 = (uint)bVar1,
     -1 < (char)bVar1)) {
    cVar6 = '\0';
    cVar4 = DAT_00ffdf84 < '\0';
    bVar5 = DAT_00ffdf84 == '\0';
    if (!(bool)cVar4) {
      uVar2 = FUN_00017c3c();
      if (bVar5 || cVar6 != cVar4) {
        return 0xffffffff;
      }
    }
    if (((DAT_00fff0a0 < '\x01') || ((char)((char)uVar2 + -7) < '\0')) ||
       (-1 < (char)((char)uVar2 + -0x11))) {
      sVar3 = (short)uVar2 << 1;
    }
    else {
      sVar3 = (short)uVar2 * 2 + 1;
    }
    return (uint)(byte)(&DAT_000e31a7)[CONCAT22((short)(uVar2 >> 0x10),sVar3)];
  }
  return 0x45;
}


