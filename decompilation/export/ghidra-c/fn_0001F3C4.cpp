// Ghidra headless export
// Entry: 0x0001F3C4
// Name: FUN_0001f3c4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000C24 wait_for_vblank_frames
//   0x00000D94 wait_for_controller_event
//   0x00001232 FUN_00001232
//   0x0000195C sample_controller_input
//   0x0000C134 normalize_source_text_line
//   0x0000C168 FUN_0000c168
//   0x0001F5A8 FUN_0001f5a8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

int * FUN_0001f3c4(void)

{
  int *in_A0;
  int *extraout_A0;
  int *extraout_A0_00;
  int *extraout_A0_01;
  undefined4 *extraout_A0_02;
  int *extraout_A0_03;
  
  do {
    if (DAT_00fff10c == '\0') {
      FUN_0001f5a8();
      normalize_source_text_line();
      DAT_00ffb740 = 0xff;
      FUN_0000c168();
      in_A0 = extraout_A0;
    }
    else {
      DAT_00fff10c = '\0';
    }
    if (*in_A0 < 0) {
      wait_for_vblank_frames();
      wait_for_controller_event();
      return (int *)((int)&VectorsTable_00000000.Reserv3E + 3);
    }
    normalize_source_text_line();
    DAT_00ffb740 = 0xff;
    FUN_0000c168();
    FUN_00001232();
    if (*extraout_A0_00 == 0) {
      _DAT_00fff0d2 = -1;
    }
    else {
      _DAT_00fff0d2 = 0;
      normalize_source_text_line();
      DAT_00ffb740 = 0xff;
      FUN_0000c168();
      FUN_00001232();
      if (*extraout_A0_01 == 0) {
        _DAT_00fff0d2 = CONCAT11(DAT_00fff0d2,0xff);
      }
      else {
        _DAT_00fff0d2 = (ushort)DAT_00fff0d2 << 8;
        normalize_source_text_line();
        DAT_00ffb740 = 0xff;
        FUN_0000c168();
        FUN_00001232();
      }
    }
    wait_for_vblank_frames();
    do {
      while( true ) {
        wait_for_vblank();
        latch_controller_events();
        sample_controller_input();
        if ((DAT_00ffdd22._0_1_ & 0x40) != 0) {
          in_A0 = (int *)*extraout_A0_02;
          goto LAB_0001f530;
        }
        if ((DAT_00ffdd22._0_1_ & 0x10) == 0) break;
        if (-1 < _DAT_00fff0d2) {
          in_A0 = (int *)extraout_A0_02[1];
          goto LAB_0001f530;
        }
      }
    } while (((DAT_00ffdd22._0_1_ & 0x20) == 0) || (DAT_00fff0d3 < '\0'));
    in_A0 = (int *)extraout_A0_02[2];
LAB_0001f530:
    if ((int)in_A0 + -0xfa < 0) {
      if (-1 < (int)in_A0 + -0xfa) {
        return in_A0;
      }
      (**(code **)((int)&PTR_DAT_0012ccb4 + (int)(short)((short)in_A0 << 2)))();
      in_A0 = extraout_A0_03;
    }
  } while( true );
}


