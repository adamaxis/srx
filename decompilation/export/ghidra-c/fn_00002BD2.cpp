// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00002BD2


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00002bd2(void)

{
  short sVar1;
  
  FUN_0000131c();
  if (_DAT_00ffdf8e == 0) {
    _DAT_00ffddc6 = _DAT_00ffddc6 & 0xff80;
    _DAT_00ffddc8 = _DAT_00ffddc8 & 0xff80;
    _DAT_00ffdf36 = 0xffff;
    _DAT_00ffdf38 = 0xffff;
    FUN_00002900();
    FUN_0000286e();
    sVar1 = _DAT_00ffe11a;
    if ((_DAT_00ffe11a != 0) && (_DAT_00ffe11a = 0, DAT_00ffe16e < '\0')) {
      (&DAT_00ff1700)[sVar1] = ((&DAT_00ff1700)[sVar1] & 0xe0) + 1;
    }
    FUN_00002c6c();
    FUN_0000301e();
    return;
  }
  _DAT_00ffdf8e = 0;
  _DAT_00ffdf8a = 0xffff;
  _DAT_00ffe172 = 0xffff;
  FUN_00003074();
  FUN_00002900();
  FUN_00002b34();
  FUN_00002c6c();
  FUN_0000301e();
  FUN_0000286e();
  return;
}


