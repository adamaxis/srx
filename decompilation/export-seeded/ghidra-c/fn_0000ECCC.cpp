// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000ECCC


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000eccc(void)

{
  byte bVar1;
  int iVar2;
  int in_A0;
  int extraout_A0;
  int in_A1;
  int extraout_A1;
  
  if ((*(byte *)(in_A1 + 0x9e) & 0x10) == 0) {
    if ((*(byte *)(in_A0 + 0x9e) & 0x10) == 0) goto LAB_0000ed3c;
    bVar1 = *(byte *)(in_A1 + 0x10);
    iVar2 = in_A0;
  }
  else {
    bVar1 = *(byte *)(in_A0 + 0x10);
    iVar2 = in_A1;
  }
  if (((bVar1 & 2) != 0) && (_DAT_00ffe128 = iVar2, DAT_00ffe176 == '\0')) {
    _DAT_00ffdd2c = 0x21174;
    FUN_0001e2e0();
    in_A0 = extraout_A0;
    in_A1 = extraout_A1;
    if ((short)(_DAT_00ffddc4 + -10) == 0) {
      DAT_00fff0ae = 4;
      if (DAT_00fffbf9 == '\x06') {
        DAT_00fff0ae = 2;
      }
    }
    else {
      DAT_00fff0ae = (&LAB_0000eefe)[(short)(_DAT_00ffddc4 + -10)];
    }
  }
LAB_0000ed3c:
  if ((*(byte *)(in_A1 + 0x9e) & 8) == 0) {
    if ((*(byte *)(in_A0 + 0x9e) & 8) == 0) {
      return;
    }
    if ((*(byte *)(in_A0 + 0xb0) & 0x80) != 0) {
      return;
    }
    if ((*(byte *)(in_A1 + 0x10) & 2) == 0) {
      return;
    }
    *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) | 0x80;
  }
  else {
    if (((*(byte *)(in_A1 + 0xb0) & 0x80) != 0) || ((*(byte *)(in_A0 + 0x10) & 2) == 0)) {
      return;
    }
    *(byte *)(in_A1 + 0xb0) = *(byte *)(in_A1 + 0xb0) | 0x80;
  }
  _DAT_00fffc3c = _DAT_00ffe110;
  _DAT_00ffdd2c = 0x1f146;
  if (DAT_00ffe180 == '\0') {
    if (DAT_00ffe182 != '\0') {
      DAT_00ffe182 = '\0';
      _DAT_00ffdd2c = 0x54402;
    }
  }
  else {
    DAT_00ffe180 = '\0';
    _DAT_00ffdd2c = 0x53c90;
  }
  FUN_0001e2ec();
  return;
}


