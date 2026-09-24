// Ghidra headless export
// Entry: 0x0005AF62
// Name: FUN_0005af62
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00001406 FUN_00001406
//   0x0000195C sample_controller_input
//   0x0000C0C2 FUN_0000c0c2
//   0x0005AF2A FUN_0005af2a
//   0x0005B29A FUN_0005b29a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005af62(void)

{
  short sVar1;
  short unaff_D7w;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined1 uVar4;
  
  DAT_00ffdd08 = 0;
  DAT_00ffdd0c = 0;
  DAT_00ffdd0a = 0;
  DAT_00ffdd0e = 0;
  DAT_00ffef14 = DAT_00ffef14 | 0xc;
  FUN_0005b29a();
  FUN_00001406();
  FUN_0000c0c2();
  DAT_00ffdd32 = &DAT_0005add0;
  DAT_00ffdd36 = DAT_0005add0;
  sVar1 = 0x1f;
  puVar2 = &DAT_00ffd8fe;
  puVar3 = &DAT_00ffd87e;
  do {
    *puVar3 = *puVar2;
    uVar4 = 0;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  sample_controller_input();
  while( true ) {
    wait_for_vblank();
    sample_controller_input();
    latch_controller_events();
    FUN_0005af2a();
    if ((bool)uVar4) {
      return;
    }
    uVar4 = 0;
    if (_DAT_00ffddc2 != 0) break;
    unaff_D7w = unaff_D7w + -1;
    if (unaff_D7w == -1) {
      return;
    }
  }
  return;
}


