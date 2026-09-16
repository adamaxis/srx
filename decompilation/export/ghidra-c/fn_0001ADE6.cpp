// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001ADE6


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ade6(void)

{
  short unaff_D7w;
  undefined1 uVar1;
  
  _DAT_00ff1596 = 0x120;
  _DAT_00ff1598 = 0xd0;
  FUN_0001b5a6();
  _DAT_00ffdf8c = 0;
  _DAT_00ffe080 = 0;
  _DAT_00ffe084 = 0;
  _DAT_00ffe0f6 = 0;
  _DAT_00ffe0aa = 0;
  _DAT_00ffe0a8 = 0;
  _DAT_00ffe058 = 0;
  _DAT_00ffe076 = 0;
  _DAT_00ffe0d4 = 0;
  _DAT_00ffe0f4 = 0;
  _DAT_00ffe082 = 0;
  _DAT_00ffe0ac = 0;
  _DAT_00ffe0f2 = 0;
  _DAT_00ffe0b6 = 0;
  _DAT_00ffe0b8 = 0;
  _DAT_00ffe0c2 = 0;
  _DAT_00ffe0d6 = 0;
  _DAT_00ffe0ee = 0;
  _DAT_00fff11e = 0;
  DAT_00ff1592 = 0;
  _DAT_00ffe05e = 0;
  _DAT_00ffe06e = 0;
  DAT_00ff15a4 = 0;
  DAT_00ffe189 = DAT_00fff0a0;
  DAT_00ffe16d = '\0';
  FUN_00026958();
  FUN_0001a908();
  FUN_0001b680();
  do {
    FUN_00000bd2();
    FUN_00000bf2();
    FUN_0000195c();
    FUN_00017fda();
    uVar1 = 0;
    if (_DAT_00ffdf8c == 0) {
      uVar1 = 0;
      if (_DAT_00ffe080 != 0) {
        FUN_0001d280();
        goto LAB_0001aedc;
      }
      if (_DAT_00ffe084 == 0) {
        if ((DAT_00fffbca == '\0') || (*(char *)(_DAT_00fffc04 + 0x4c) == '\0')) {
          _DAT_00ffe084 = 1;
          _DAT_00ffe086 = 0;
        }
      }
      else {
        FUN_0001b302();
        if (_DAT_00ffe084 == 0) {
          FUN_00018776();
          FUN_00026a00();
          return;
        }
      }
      FUN_00019788();
      FUN_0001c754();
      FUN_00019432();
      FUN_0001a370();
      if (DAT_00fff0b1 < '\0') {
        return;
      }
      if ((DAT_00ffe16d != '\0') && (DAT_00ffe16d = DAT_00ffe16d + -1, DAT_00ffe16d == '\0')) {
        FUN_000530de();
      }
      if (_DAT_00ffe0aa == 0) goto LAB_0001afc0;
      FUN_0001c0c2();
    }
    else {
LAB_0001aedc:
      FUN_0001d2e6();
      if ((bool)uVar1) {
        _DAT_00ffdf8c = 0;
        FUN_00053126();
        FUN_000530ea();
        if (DAT_00fff0a9 == '\0') {
          FUN_00054662();
        }
        else {
          FUN_00053704();
        }
        FUN_00053126();
        return;
      }
LAB_0001afc0:
      FUN_0001bf7a();
    }
    _DAT_00ffdd02 = 1;
    if (_DAT_00ffdf8c == 0) {
      if (_DAT_00ffe076 == 4) {
        FUN_0001a7be();
      }
      if (_DAT_00ffe0f6 != 0) {
        FUN_0001b218();
      }
      FUN_00026972();
      FUN_0001c3fa();
      if ((DAT_00ff1592 & 1) != 0) {
        FUN_0001c61c();
      }
      if ((DAT_00ff1592 & 2) != 0) {
        FUN_0001c1f6();
      }
      if ((DAT_00ff1592 & 4) != 0) {
        FUN_0001b45c();
      }
      if ((DAT_00ff1592 & 0x10) != 0) {
        FUN_0001d1f4();
      }
      if ((DAT_00ff1592 & 0x40) != 0) {
        FUN_0001cd5a();
      }
      if ((DAT_00ff1592 & 0x20) != 0) {
        FUN_0001b87a();
      }
      if (_DAT_00ffe0ac != 0) {
        FUN_0001ba6e();
      }
      if ((DAT_00fff0a1 & 1) != 0) {
        FUN_0001bb6a();
      }
      if ((DAT_00ff1592 & 0x80) != 0) {
        FUN_0001beba();
      }
      if (_DAT_00ffe058 != 0) {
        FUN_0001bce8();
      }
      if (_DAT_00ffe05e != 0) {
        FUN_0001c866();
      }
      if (_DAT_00ffe06e != 0) {
        FUN_0001ca12();
      }
      if (_DAT_00ffe076 != 0) {
        FUN_0001a5a0();
      }
      if (_DAT_00ffe0d4 != 0) {
        FUN_0001a48a();
      }
      if (_DAT_00ffe0de != 0) {
        FUN_0001b282();
      }
      FUN_0001ccdc();
      FUN_0001cc24();
    }
    else {
      FUN_0001b364();
    }
    if (_DAT_00ffe080 == 0) {
      FUN_0001aa40();
    }
    FUN_000268e6();
    _DAT_00ffd980 = 1;
    if ((_DAT_00ffdf8c == 0) && ((DAT_00ffdd22 & 0x80) != 0)) {
      FUN_00053126();
      FUN_000530ea();
      FUN_0001b5a6();
      FUN_0001822c();
      if (DAT_00fff0b1 != '\0') {
        return;
      }
      FUN_000176ba();
      if (unaff_D7w == 0) {
        return;
      }
      FUN_000186fa();
      FUN_0001789c();
      _DAT_00c00004 = 0x8174;
      FUN_00053126();
      FUN_000530ea();
    }
    DAT_00ffef14 = DAT_00ffef14 | 2;
  } while( true );
}


