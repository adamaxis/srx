// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x000101FE


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_000101fe(void)

{
  short sVar1;
  
  sVar1 = ((short)CONCAT31((int3)((uint)DAT_00fff0d1 * 7 >> 8),
                           DAT_00fff0d0 + (char)((uint)DAT_00fff0d1 * 7)) + 1) * 2;
  _DAT_00fff098 = (ushort)*(byte *)(_DAT_00fff10e + (short)(sVar1 + 1));
  if ((*(char *)(_DAT_00fffc04 + 0x56) < '\0') &&
     ((char)(*(char *)(_DAT_00fff10e + sVar1) + '\x01') == *(char *)(_DAT_00fffc04 + 0x57))) {
    _DAT_00fff096 = (ushort)*(byte *)(_DAT_00fffc04 + 0x9d);
    return;
  }
  _DAT_00fff096 = _DAT_00fff098;
  return;
}


