// Ghidra headless export
// Entry: 0x00001406
// Name: FUN_00001406
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000CEC FUN_00000cec
//   0x00000E34 FUN_00000e34
//   0x00000F5C FUN_00000f5c
//   0x00000FFE FUN_00000ffe
//   0x0000102A FUN_0000102a
//   0x00001232 FUN_00001232
//   0x0000131C step_palette_transition
//   0x00001368 FUN_00001368
//   0x00001530 FUN_00001530
//   0x00001728 FUN_00001728


void FUN_00001406(void)

{
  uint uVar1;
  int unaff_A6;
  uint *puVar2;
  uint *puVar3;
  
  step_palette_transition();
  FUN_00000cec();
  FUN_00000ffe();
  FUN_0000102a();
  FUN_00000e34();
  FUN_00001368();
  DAT_00ffdd08 = *(undefined2 *)(unaff_A6 + 8);
  DAT_00ffdd0c = *(undefined2 *)(unaff_A6 + 10);
  DAT_00ffdd0a = *(undefined2 *)(unaff_A6 + 0xc);
  puVar2 = (uint *)(unaff_A6 + 0x10);
  DAT_00ffdd0e = *(undefined2 *)(unaff_A6 + 0xe);
  while( true ) {
    puVar3 = puVar2 + 1;
    if (*puVar2 == 0) break;
    if ((*puVar2 & 1) == 0) {
      puVar2 = puVar2 + 3;
      FUN_00000f5c();
    }
    else {
      puVar2 = puVar2 + 3;
      FUN_00001530();
    }
  }
  while (uVar1 = *puVar3, uVar1 != 0) {
    puVar3 = (uint *)((int)puVar3 + 0xe);
    if ((uVar1 & 1) == 0) {
      FUN_00001232();
    }
    else {
      FUN_00001728();
    }
  }
  DAT_00ffef14 = DAT_00ffef14 | 0xd;
  wait_for_vblank();
  return;
}


