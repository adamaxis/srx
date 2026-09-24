// Ghidra headless export
// Entry: 0x00026A00
// Name: FUN_00026a00
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 wait_for_vblank
//   0x00000BF2 latch_controller_events
//   0x00000F5C FUN_00000f5c
//   0x00001358 FUN_00001358
//   0x0000195C sample_controller_input
//   0x00010952 FUN_00010952
//   0x00026BFE FUN_00026bfe
//   0x00026C26 FUN_00026c26
//   0x000530DE FUN_000530de


void FUN_00026a00(void)

{
  short extraout_D1w;
  short sVar1;
  short extraout_D1w_00;
  undefined4 uVar2;
  uint unaff_D3;
  uint uVar3;
  int iVar4;
  int iVar5;
  undefined **ppuVar6;
  int iVar7;
  undefined2 uVar8;
  
  DAT_00fff0b1 = 0xff;
  FUN_000530de();
  FUN_00000f5c();
  FUN_00010952();
  FUN_00001358();
  DAT_00ffef14 = DAT_00ffef14 | 1;
  uVar2 = 0x6000;
  FUN_00026bfe();
  FUN_00026c26();
  uVar3 = unaff_D3 & 0xffff0000;
  FUN_00026bfe();
  sVar1 = 0xd;
  iVar4 = 0xe6fe;
  iVar5 = 0xe752;
  ppuVar6 = &PTR_DAT_0000dfa8;
  iVar7 = 0xeea8;
  do {
    wait_for_vblank(sVar1,uVar2,uVar3,iVar4,iVar5,ppuVar6,iVar7);
    uVar8 = (undefined2)((uint)uVar2 >> 0x10);
    latch_controller_events();
    sample_controller_input();
    iVar4 = iVar4 + 2;
    uVar3 = uVar3 & 0xffff0000;
    uVar2 = CONCAT22(uVar8,0x6000);
    FUN_00026bfe();
    iVar5 = iVar5 + -2;
    FUN_00026bfe();
    ppuVar6 = ppuVar6 + 0x20;
    FUN_00026c26();
    iVar7 = iVar7 + -0x80;
    FUN_00026c26();
    sVar1 = extraout_D1w + -1;
  } while (sVar1 != -1);
  sVar1 = 5;
  do {
    wait_for_vblank(sVar1,uVar2,uVar3,iVar4,iVar5);
    uVar8 = (undefined2)((uint)uVar2 >> 0x10);
    latch_controller_events();
    sample_controller_input();
    iVar4 = iVar4 + 2;
    uVar3 = uVar3 & 0xffff0000;
    uVar2 = CONCAT22(uVar8,0x6000);
    FUN_00026bfe();
    iVar5 = iVar5 + -2;
    FUN_00026bfe();
    sVar1 = extraout_D1w_00 + -1;
  } while (sVar1 != -1);
  FUN_00026bfe();
  sVar1 = 10;
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_00026bfe();
  sVar1 = 5;
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_00026bfe();
  sVar1 = 5;
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  FUN_00026bfe();
  sVar1 = 10;
  do {
    wait_for_vblank();
    latch_controller_events();
    sample_controller_input();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


