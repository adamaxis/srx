// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000391A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000391a(void)

{
  byte *pbVar1;
  undefined1 *puVar2;
  undefined1 *puVar3;
  
  DAT_00fff0cc = '\x01';
  FUN_000530de();
  puVar2 = _DAT_00fffc04;
  FUN_000033aa();
  puVar3 = _DAT_00fffc04;
  if (*(int *)(_DAT_00fffc04 + 0xa2) != 0) {
    pbVar1 = (byte *)(*(int *)(_DAT_00fffc04 + 0xa2) + 0x11);
    *pbVar1 = *pbVar1 & 0xef;
  }
  do {
    if (*(short *)(puVar3 + 0x3e) == 2) {
      DAT_00fff0cc = DAT_00fff0cc + -1;
      puVar3[0x10] = puVar3[0x10] & 0xfd;
      _DAT_00fffc04 = &DAT_00ff0100;
      if (DAT_00fff0cc < '\0') break;
    }
    else {
      puVar3[0x10] = puVar3[0x10] & 0xfd;
      _DAT_00fffc04 = _DAT_00fffc04 + 0x100;
    }
    puVar3 = _DAT_00fffc04;
    _DAT_00fffc04[0x10] = _DAT_00fffc04[0x10] | 2;
  } while (((puVar3[0x10] & 1) == 0) ||
          ((DAT_00fff09d == '\0' && (((puVar3[0x10] & 0x40) != 0 || ((puVar3[0x10] & 0x10) != 0)))))
          );
  DAT_00fff0cc = 0;
  if (puVar3 != puVar2) {
    FUN_0001e540();
  }
  FUN_0001dc4c();
  FUN_000033aa();
  FUN_00003548();
  FUN_000038fa();
  return;
}


