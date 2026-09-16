// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001CD5A


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001cd5a(void)

{
  ushort uVar1;
  undefined2 *extraout_A0;
  undefined **ppuVar2;
  undefined8 uVar3;
  
  if (DAT_00ff15cc == '\0') {
    return;
  }
  cRam00ff15d1 = cRam00ff15d1 + -1;
  if ((cRam00ff15d1 < '\0') && (DAT_00ff15cc = '\0', _DAT_00ffe0a8 != 0)) {
    _DAT_00ffe0aa = 10;
  }
  cRam00ff15ca = cRam00ff15ca + -1;
  if (cRam00ff15ca < '\0') {
    cRam00ff15ca = '\x14';
    if ((char)(cRam00ff15cb + '\x01') < '\x03') {
      cRam00ff15cb = cRam00ff15cb + '\x01';
    }
  }
  cRam00ff15d0 = cRam00ff15d0 + -1;
  if (cRam00ff15d0 < '\0') {
    cRam00ff15d0 = '\x02';
    cRam00ff15cf = cRam00ff15cf + '\x01';
    if ('\x02' < cRam00ff15cf) {
      cRam00ff15cf = '\0';
    }
  }
  ppuVar2 = &PTR_DAT_0001d158;
  if ((cRam00ff15cb != '\0') && (ppuVar2 = &PTR_DAT_0001d140, cRam00ff15cb != '\x01')) {
    ppuVar2 = &PTR_DAT_0001d14c;
  }
  sRam00ff15c6 = sRam00ff15c6 + -8;
  uRam00ff15c8 = uRam00ff15c8 - 0xe;
  uVar1 = uRam00ff15c8 >> 4;
  uVar3 = FUN_00001378(*(undefined4 *)((int)ppuVar2 + (int)(short)((short)cRam00ff15cf << 2)));
  if (*(char *)(extraout_A0 + 3) != '\0') {
    if (*(char *)(extraout_A0 + 2) == '\0') {
      if (*(char *)((int)extraout_A0 + 5) == '\0') {
        uRam00ffe088 = *extraout_A0;
        uRam00ffe08a = *extraout_A0;
        uRam00ffe08c = extraout_A0[1];
        uRam00ffe08e = extraout_A0[1];
      }
      else if (*(char *)((int)extraout_A0 + 5) == '\x01') {
        uRam00ffe090 = *extraout_A0;
        uRam00ffe092 = extraout_A0[1];
        uRam00ffe094 = *extraout_A0;
        uRam00ffe096 = extraout_A0[1];
        uRam00ffe098 = uRam00ffe088;
        uRam00ffe09a = uRam00ffe08c;
        uRam00ffe09c = uRam00ffe088;
        uRam00ffe09e = uRam00ffe08c;
        uRam00ffe0a0 = uRam00ffe08a;
        uRam00ffe0a2 = uRam00ffe08e;
        uRam00ffe0a4 = uRam00ffe08a;
        uRam00ffe0a6 = uRam00ffe08e;
      }
    }
    if (*(char *)((int)extraout_A0 + 5) != '\0') {
      if (*(char *)((int)extraout_A0 + 5) != '\x01') {
        func_0x0001cf4a();
      }
      func_0x0001cefa();
    }
    return;
  }
  DAT_00ff1592 = DAT_00ff1592 & 0xbf;
  _DAT_00ffe0d6 = 0;
  FUN_00017c84(uVar3,uVar1);
  FUN_00017c52();
  if ('\0' < DAT_00fff0d7) {
    FUN_000564d6();
    FUN_00017998();
  }
  FUN_0001b706();
  return;
}


