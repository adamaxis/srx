// Ghidra headless export
// Entry: 0x00059C5E
// Name: FUN_00059c5e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000131C step_palette_transition
//   0x0000195C sample_controller_input
//   0x000530EA FUN_000530ea
//   0x00053126 FUN_00053126
//   0x00059D12 FUN_00059d12
//   0x00059D7C FUN_00059d7c
//   0x0005AA96 FUN_0005aa96
//   0x0005AD1A FUN_0005ad1a
//   0x0005AF62 FUN_0005af62


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00059c5e(void)

{
  DAT_00ffe18b = 0xff;
  sample_controller_input();
  _DAT_00ffddc2 = 0;
  FUN_000530ea();
  FUN_0005af62();
  if (_DAT_00ffddc2 == 0) {
    FUN_00053126();
    FUN_000530ea();
    _DAT_00ffddc2 = 0;
    FUN_00059d7c();
    if (_DAT_00ffddc2 == 0) {
      FUN_00053126();
      FUN_000530ea();
      _DAT_00ffddc2 = 0;
      FUN_0005aa96();
      if (_DAT_00ffddc2 == 0) {
        _DAT_00ffddc2 = 0;
        FUN_0005ad1a();
        _DAT_00ffddc2 = 0;
        step_palette_transition();
        DAT_00fffbf8 = 2;
        FUN_00059d12();
      }
    }
  }
  sample_controller_input();
  step_palette_transition();
  FUN_00053126();
  return;
}


