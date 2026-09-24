// Ghidra headless export
// Entry: 0x0001ADE6
// Name: FUN_0001ade6
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x0000195C sample_controller_input
//   0x000176BA FUN_000176ba
//   0x0001789C FUN_0001789c
//   0x00017FDA FUN_00017fda
//   0x0001822C FUN_0001822c
//   0x000186FA FUN_000186fa
//   0x00018776 FUN_00018776
//   0x00019432 FUN_00019432
//   0x00019788 FUN_00019788
//   0x0001A370 FUN_0001a370
//   0x0001A48A FUN_0001a48a
//   0x0001A5A0 FUN_0001a5a0
//   0x0001A7BE FUN_0001a7be
//   0x0001A908 FUN_0001a908
//   0x0001AA40 FUN_0001aa40
//   0x0001B218 FUN_0001b218
//   0x0001B282 FUN_0001b282
//   0x0001B302 FUN_0001b302
//   0x0001B364 FUN_0001b364
//   0x0001B45C FUN_0001b45c
//   0x0001B5A6 FUN_0001b5a6
//   0x0001B680 FUN_0001b680
//   0x0001B87A FUN_0001b87a
//   0x0001BA6E FUN_0001ba6e
//   0x0001BB6A FUN_0001bb6a
//   0x0001BCE8 FUN_0001bce8
//   0x0001BEBA FUN_0001beba
//   0x0001BF7A FUN_0001bf7a
//   0x0001C0C2 FUN_0001c0c2
//   0x0001C1F6 FUN_0001c1f6
//   0x0001C3FA FUN_0001c3fa
//   0x0001C61C FUN_0001c61c
//   0x0001C754 FUN_0001c754
//   0x0001C866 FUN_0001c866
//   0x0001CA12 FUN_0001ca12
//   0x0001CC24 FUN_0001cc24
//   0x0001CCDC FUN_0001ccdc
//   0x0001CD5A FUN_0001cd5a
//   0x0001D1F4 FUN_0001d1f4
//   0x0001D280 FUN_0001d280
//   0x0001D2E6 FUN_0001d2e6
//   0x000268E6 FUN_000268e6
//   0x00026958 FUN_00026958
//   0x00026972 FUN_00026972
//   0x00026A00 FUN_00026a00
//   0x000530DE service_audio_if_enabled
//   0x000530EA FUN_000530ea
//   0x00053126 FUN_00053126
//   0x00053704 FUN_00053704
//   0x00054662 FUN_00054662


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001ade6(void)

{
  short unaff_D7w;
  undefined1 uVar1;
  
  DAT_00ff1596 = 0x120;
  DAT_00ff1598 = 0xd0;
  FUN_0001b5a6();
  _DAT_00ffdf8c = 0;
  _DAT_00ffe080 = 0;
  _DAT_00ffe084 = 0;
  _DAT_00ffe0f6 = 0;
  DAT_00ffe0aa = 0;
  _DAT_00ffe0a8 = 0;
  DAT_00ffe058 = 0;
  DAT_00ffe076 = 0;
  _DAT_00ffe0d4 = 0;
  _DAT_00ffe0f4 = 0;
  _DAT_00ffe082 = 0;
  _DAT_00ffe0ac = 0;
  _DAT_00ffe0f2 = 0;
  DAT_00ffe0b6 = 0;
  DAT_00ffe0b8 = 0;
  _DAT_00ffe0c2 = 0;
  _DAT_00ffe0d6 = 0;
  _DAT_00ffe0ee = 0;
  DAT_00fff11e = 0;
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
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    FUN_00017fda();
    uVar1 = 0;
    if (_DAT_00ffdf8c == 0) {
      uVar1 = 0;
      if (_DAT_00ffe080 != 0) {
        FUN_0001d280();
        goto LAB_0001aedc;
      }
      if (_DAT_00ffe084 == 0) {
        if ((DAT_00fffbca == '\0') || (*(char *)(DAT_00fffc04 + 0x4c) == '\0')) {
          _DAT_00ffe084 = 1;
          DAT_00ffe086 = 0;
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
        service_audio_if_enabled();
      }
      if (DAT_00ffe0aa == 0) goto LAB_0001afc0;
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
    DAT_00ffdd02 = 1;
    if (_DAT_00ffdf8c == 0) {
      if (DAT_00ffe076 == 4) {
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
      if (DAT_00ffe058 != 0) {
        FUN_0001bce8();
      }
      if (_DAT_00ffe05e != 0) {
        FUN_0001c866();
      }
      if (_DAT_00ffe06e != 0) {
        FUN_0001ca12();
      }
      if (DAT_00ffe076 != 0) {
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
    if ((_DAT_00ffdf8c == 0) && ((DAT_00ffdd22._0_1_ & 0x80) != 0)) {
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
      VDP_CTRL = 0x8174;
      FUN_00053126();
      FUN_000530ea();
    }
    DAT_00ffef14 = DAT_00ffef14 | 2;
  } while( true );
}


