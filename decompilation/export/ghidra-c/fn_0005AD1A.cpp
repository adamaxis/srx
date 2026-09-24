// Ghidra headless export
// Entry: 0x0005AD1A
// Name: FUN_0005ad1a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00001406 FUN_00001406
//   0x0000195C sample_controller_input
//   0x0005A96A FUN_0005a96a
//   0x0005A9E2 FUN_0005a9e2
//   0x0005B002 FUN_0005b002
//   0x0005B00A FUN_0005b00a
//   0x0005B056 FUN_0005b056
//   0x0005B08E FUN_0005b08e
//   0x0005B0BC FUN_0005b0bc
//   0x0005B198 FUN_0005b198
//   0x0005B29A FUN_0005b29a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0005ad1a(void)

{
  short sVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  
  FUN_0005b29a();
  FUN_00001406();
  DAT_00fff11e = DAT_00ffdf88;
  FUN_0005b002();
  FUN_0005b198();
  FUN_0005b0bc();
  FUN_0005a9e2();
  sVar1 = 0x1f;
  puVar2 = &DAT_00ffd8fe;
  puVar3 = &DAT_00ffd87e;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  sample_controller_input();
  sVar1 = 0x528;
  do {
    wait_for_vblank();
    sample_controller_input();
    latch_controller_events();
    if (sVar1 == 0x1e0) {
      FUN_0005a96a();
    }
    FUN_0005b08e();
    FUN_0005b00a();
    FUN_0005b056();
    FUN_0005b0bc();
  } while ((_DAT_00ffddc2 == 0) && (sVar1 = sVar1 + -1, sVar1 != -1));
  return;
}


