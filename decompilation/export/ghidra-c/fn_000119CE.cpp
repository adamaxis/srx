// Ghidra headless export
// Entry: 0x000119CE
// Name: FUN_000119ce
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000BD2 FUN_00000bd2
//   0x00000BF2 FUN_00000bf2
//   0x00001232 FUN_00001232
//   0x0000195C FUN_0000195c
//   0x00011ABE FUN_00011abe
//   0x000530DE FUN_000530de


undefined8 FUN_000119ce(void)

{
  undefined4 uVar1;
  undefined4 extraout_D1;
  undefined4 extraout_D1_00;
  uint unaff_D2;
  uint uVar2;
  undefined4 unaff_D3;
  undefined *puVar3;
  undefined4 unaff_D5;
  undefined4 unaff_D6;
  undefined4 uVar4;
  undefined4 extraout_A0;
  undefined4 uVar5;
  undefined4 extraout_A0_00;
  undefined4 extraout_A0_01;
  undefined4 extraout_A1;
  undefined4 uVar6;
  undefined4 extraout_A1_00;
  undefined4 extraout_A1_01;
  undefined4 unaff_A2;
  undefined4 unaff_A3;
  undefined4 unaff_A4;
  undefined4 unaff_A5;
  undefined4 unaff_A6;
  undefined8 uVar7;
  
  puVar3 = &UNK_0000ca9e;
  uVar4 = 5;
  uVar2 = unaff_D2 & 0xffff0000;
  FUN_00001232();
  FUN_00011abe();
  uVar1 = extraout_D1;
  uVar5 = extraout_A0;
  uVar6 = extraout_A1;
  do {
    while( true ) {
      FUN_00000bd2(uVar1,uVar2,unaff_D3,puVar3,unaff_D5,unaff_D6,uVar4,uVar5,uVar6,unaff_A2,unaff_A3
                   ,unaff_A4,unaff_A5,unaff_A6);
      FUN_00000bf2();
      FUN_0000195c();
      if (((DAT_00ffdd22 & 0x200) != 0) && ((DAT_00ffdd22 & 8) == 0)) break;
LAB_00011a44:
      if ((DAT_00ffdd22 & 0x800) == 0) goto LAB_00011a5c;
      uVar1 = 1;
LAB_00011a70:
      FUN_000530de(uVar1);
      FUN_00011abe();
      uVar1 = extraout_D1_00;
      uVar5 = extraout_A0_00;
      uVar6 = extraout_A1_00;
    }
    if ((DAT_00ffdd22 & 4) == 0) {
      if (-1 < DAT_00fff115) goto LAB_00011a44;
      goto LAB_00011a9a;
    }
LAB_00011a5c:
    if ((DAT_00ffdd22 & 0x400) != 0) {
      uVar1 = 0xffffffff;
      goto LAB_00011a70;
    }
  } while ((DAT_00ffdd22 & 0x8000) == 0);
  DAT_00fff0b1 = 0xff;
LAB_00011a9a:
  FUN_00001232();
  uVar7 = FUN_00011ae4();
  FUN_00012cee(uVar7,uVar2 & 0xffff0000,unaff_D3,&UNK_0000ca9e,unaff_D5,unaff_D6,5,extraout_A0_01,
               extraout_A1_01,unaff_A2,unaff_A3,unaff_A4,unaff_A5,unaff_A6);
  FUN_00012cee();
  return uVar7;
}


