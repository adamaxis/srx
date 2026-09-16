// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00003074


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00003074(void)

{
  short sVar1;
  short sVar2;
  undefined2 *puVar3;
  undefined2 *puVar4;
  
  if (_DAT_00ffe108 == 0) {
    FUN_0000c7f8();
  }
  DAT_00fffbf8 = (undefined1)_DAT_00ffddc4;
  sVar1 = FUN_00003032();
  sVar2 = 0x2f;
  puVar3 = *(undefined2 **)((int)&PTR_DAT_00003232 + (int)(short)(sVar1 << 2));
  puVar4 = (undefined2 *)&DAT_00ffddc6;
  do {
    *puVar4 = *puVar3;
    sVar2 = sVar2 + -1;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  } while (sVar2 != -1);
  _DAT_00ffde22 = _DAT_00ffddde;
  if ((_DAT_00fffc0e != 0) && (_DAT_00ffe10e == 0)) {
    _DAT_00ffde08 = _DAT_00ffe10a;
    _DAT_00ffde0c = _DAT_00ffe10a;
    _DAT_00ffde10 = _DAT_00ffe10a;
    _DAT_00ffde0a = _DAT_00ffe10c;
    _DAT_00ffde0e = _DAT_00ffe10c;
    _DAT_00ffde12 = _DAT_00ffe10c;
    _DAT_00ffddc6 = _DAT_00ffe10a - 0xa00;
    if (_DAT_00ffe10a < 0xa00) {
      _DAT_00ffddc6 = 0;
    }
    _DAT_00ffddc8 = _DAT_00ffe10c - 0x700;
    if (_DAT_00ffe10c < 0x700) {
      _DAT_00ffddc8 = 0;
    }
  }
  _DAT_00ffe10e = 0;
  sVar1 = 0xe0f;
  puVar3 = _DAT_00ffddda;
  puVar4 = (undefined2 *)&DAT_00ff2700;
  do {
    *puVar4 = *puVar3;
    sVar1 = sVar1 + -1;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  } while (sVar1 != -1);
  sVar1 = 0xe0f;
  puVar3 = _DAT_00ffdde2;
  puVar4 = (undefined2 *)&DAT_00ff4700;
  do {
    *puVar4 = *puVar3;
    sVar1 = sVar1 + -1;
    puVar3 = puVar3 + 1;
    puVar4 = puVar4 + 1;
  } while (sVar1 != -1);
  return;
}


