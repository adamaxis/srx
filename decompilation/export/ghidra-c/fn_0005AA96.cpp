// Ghidra headless export
// Entry: 0x0005AA96
// Name: FUN_0005aa96
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000E34 FUN_00000e34
//   0x00000FFE FUN_00000ffe
//   0x0000102A FUN_0000102a
//   0x000012F6 FUN_000012f6
//   0x00001368 copy_target_palette
//   0x00001530 FUN_00001530
//   0x00001728 FUN_00001728
//   0x0000195C sample_controller_input
//   0x0000C0C2 FUN_0000c0c2
//   0x0005AA5A FUN_0005aa5a
//   0x0005AB96 FUN_0005ab96
//   0x0005ABD2 FUN_0005abd2
//   0x0005AC28 FUN_0005ac28
//   0x0005AC70 FUN_0005ac70
//   0x0005B32C FUN_0005b32c


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005aa96(void)

{
  undefined2 uVar1;
  short sVar2;
  
  FUN_00000e34();
  FUN_00000ffe();
  FUN_0000102a();
  FUN_00001530();
  FUN_00001530();
  FUN_0000c0c2();
  FUN_0005aa5a();
  wait_for_vblank();
  copy_target_palette();
  FUN_0005b32c();
  DAT_00fff11e = DAT_00ffdf88;
  FUN_0005ac28();
  FUN_0005ac70();
  FUN_0005abd2();
  wait_for_vblank();
  FUN_00001728();
  uVar1 = DAT_00ffdf88;
  FUN_000012f6();
  DAT_00ffdf88 = uVar1;
  sample_controller_input();
  sVar2 = 0x114;
  do {
    wait_for_vblank();
    sample_controller_input();
    latch_controller_events();
    FUN_0005abd2();
    FUN_0005ab96();
    if (_DAT_00ffddc2 != 0) {
      return;
    }
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


