// Ghidra headless export
// Entry: 0x00001A9E
// Name: run_title_loop
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000E34 FUN_00000e34
//   0x0000131C step_palette_transition
//   0x0000195C sample_controller_input
//   0x00001BAC FUN_00001bac
//   0x00001BEA FUN_00001bea
//   0x00001C1E FUN_00001c1e
//   0x00001C6A FUN_00001c6a
//   0x00001D7A FUN_00001d7a
//   0x00001DCA FUN_00001dca
//   0x00001DE0 FUN_00001de0
//   0x00001E4E FUN_00001e4e
//   0x00002634 FUN_00002634
//   0x000026D0 FUN_000026d0


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void run_title_loop(void)

{
  ushort uVar1;
  
  DAT_00fff101 = 0;
  FUN_00001d7a();
  DAT_00ffe00a = 0;
  DAT_00ffdd0e = 0;
  sample_controller_input();
  _DAT_00ffddc2 = 0;
  _DAT_00ffdd2c = 0x5a;
  DAT_00ffdd2e = 0;
  _DAT_00ffdd3c = 0;
  DAT_00ffdd5a = 0;
  DAT_00ffdd5c = 0x5a;
  DAT_00ffdd42 = 0x98;
  DAT_00ffdd40 = 0;
  DAT_00ffdd46 = 0;
  uVar1 = 0xe10;
  do {
    wait_for_vblank();
    FUN_00001c1e();
    FUN_00002634();
    FUN_000026d0();
    sample_controller_input();
    FUN_00001bea();
    latch_controller_events();
    _DAT_00ffdd2c = _DAT_00ffdd2c + -1;
    if ((_DAT_00ffdd2c < 0) && (DAT_00fff1e6 < 0x26)) {
      DAT_00ffdd40 = DAT_00ffdd40 + -1;
      if (DAT_00ffdd40 < 0) {
        DAT_00ffdd40 = 1;
        FUN_00001e4e();
      }
      FUN_00001c6a();
      DAT_00ffef14 = DAT_00ffef14 | 2;
    }
    else {
      FUN_00000e34();
    }
    if (uVar1 < 0xc6c) {
      if ((uVar1 & 0x3f) < 10) {
        FUN_00001de0();
      }
      else {
        FUN_00001dca();
      }
      if (_DAT_00ffddc2 != 0) break;
    }
    uVar1 = uVar1 - 1;
  } while (uVar1 != 0xffff);
  sample_controller_input();
  step_palette_transition();
  FUN_00001bac();
  return;
}


