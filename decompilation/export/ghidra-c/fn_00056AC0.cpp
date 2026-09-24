// Ghidra headless export
// Entry: 0x00056AC0
// Name: FUN_00056ac0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000D94 FUN_00000d94
//   0x00000E34 FUN_00000e34
//   0x00000F5C FUN_00000f5c
//   0x00001530 decode_and_upload_source_stream
//   0x0000195C sample_controller_input
//   0x0000334C FUN_0000334c
//   0x00003404 FUN_00003404
//   0x000039F2 FUN_000039f2
//   0x0000C0C2 load_title_font
//   0x0000C0DA render_title_text_source
//   0x0000C406 FUN_0000c406
//   0x0000D4C8 FUN_0000d4c8
//   0x00010BC4 FUN_00010bc4
//   0x00011048 FUN_00011048
//   0x000110EC FUN_000110ec
//   0x000530DE FUN_000530de
//   0x000566E2 FUN_000566e2
//   0x000566EC FUN_000566ec
//   0x000566F6 FUN_000566f6
//   0x0005670E FUN_0005670e
//   0x00056DC4 FUN_00056dc4
//   0x000587DA FUN_000587da
//   0x00058B5E FUN_00058b5e
//   0x00058BB6 FUN_00058bb6
//   0x00058C54 FUN_00058c54
//   0x00058CDE FUN_00058cde
//   0x00058D32 FUN_00058d32


void FUN_00056ac0(void)

{
  DAT_00fff0b1 = 0;
  VDP_CTRL = 0x9100;
  FUN_00010bc4();
  FUN_00011048();
  FUN_0005670e();
  if (DAT_00fff0b7 == '\x01') {
    FUN_00000f5c();
  }
  if (DAT_00fff0b7 == '\x02') {
    FUN_000566e2();
    FUN_000566f6();
    decode_and_upload_source_stream();
  }
  load_title_font();
  render_title_text_source();
LAB_00056b66:
  if (DAT_00fff0b7 == '\0') {
    DAT_00fff10e = &DAT_00fff9fa + (uint)*(ushort *)(DAT_00fffc04 + 0xa0) * 0x11;
  }
  FUN_00000e34();
  DAT_00fff112 = '\0';
  DAT_00fff0d0 = '\0';
  DAT_00fff0b8 = '\0';
  DAT_00fff0c2._0_1_ = -1;
  FUN_000566ec();
  FUN_000110ec();
  do {
    DAT_00fff0d1 = '\0';
    FUN_0000c406();
    if ((DAT_00fff0b7 == '\x02') && ((DAT_00ff018f & 1) == 0)) {
      render_title_text_source();
      FUN_00000d94();
LAB_00056da8:
      FUN_0000334c();
      FUN_000039f2();
      FUN_00003404();
      VDP_CTRL = 0x9191;
      return;
    }
    if (DAT_00fff0b7 == '\x01') {
      DAT_00fff0d1 = '\x01';
      FUN_00058d32();
    }
    else {
      FUN_00058b5e();
      FUN_00058bb6();
    }
    FUN_000587da();
LAB_00056c54:
    while( true ) {
      wait_for_vblank();
      latch_controller_events();
      sample_controller_input();
      if ((DAT_00ffdd22._0_1_ & 4) == 0) break;
      FUN_000530de();
LAB_00056cb6:
      FUN_00058c54();
    }
    if ((DAT_00ffdd22._0_1_ & 1) != 0) {
      FUN_000530de();
LAB_00056cd4:
      if (DAT_00fff0b7 != '\x01') {
        FUN_00058cde();
      }
      goto LAB_00056c54;
    }
    if ((DAT_00ffdd22._0_1_ & 8) != 0) {
      FUN_000530de();
      goto LAB_00056cb6;
    }
    if ((DAT_00ffdd22._0_1_ & 2) != 0) {
      FUN_000530de();
      goto LAB_00056cd4;
    }
    if (((DAT_00ffdd22._0_1_ & 0x40) == 0) && ((DAT_00ffdd22._0_1_ & 0x10) == 0)) {
      if ((DAT_00ffdd22._0_1_ & 0x20) != 0) {
        FUN_000530de();
        FUN_0000d4c8();
        goto LAB_00056b66;
      }
      if ((DAT_00ffdd22._0_1_ & 0x80) != 0) goto LAB_00056da8;
      goto LAB_00056c54;
    }
    FUN_000530de();
    if (DAT_00fff0d1 == '\0') {
      if (((DAT_00fff0d0 == '\x03') && (DAT_00fff0b7 < '\0')) && (DAT_00fff0c2._0_1_ < '\0')) {
        render_title_text_source();
      }
      else {
        FUN_000587da();
      }
      goto LAB_00056c54;
    }
    FUN_00056dc4();
    if (DAT_00fff112 < '\0') goto LAB_00056c54;
    FUN_00000e34();
    DAT_00fff0d0 = DAT_00fff0b8;
  } while( true );
}


