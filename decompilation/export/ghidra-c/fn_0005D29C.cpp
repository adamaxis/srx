// Ghidra headless export
// Entry: 0x0005D29C
// Name: FUN_0005d29c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00005A56 project_entity_screen_position
//   0x0001E2E0 FUN_0001e2e0
//   0x0005BA66 FUN_0005ba66
//   0x0005D3E6 FUN_0005d3e6
//   0x0005D416 FUN_0005d416
//   0x0005D55C FUN_0005d55c
//   0x0005D57E FUN_0005d57e
//   0x0005D5A2 FUN_0005d5a2
//   0x0005D5FC FUN_0005d5fc
//   0x0005D65A FUN_0005d65a


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined8 FUN_0005d29c(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short sVar1;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  undefined *puVar2;
  int extraout_A1;
  undefined8 uVar3;
  
  if (((DAT_00ffe186 == '\x02') || (uVar3 = CONCAT44(in_D0,in_D1), DAT_00ffe186 == '\x03')) &&
     (uVar3 = CONCAT44(in_D0,in_D1), -1 < DAT_00fff13e._2_2_)) {
    if (DAT_00fff13e._2_2_ == 2) {
      project_entity_screen_position();
      FUN_0005d55c();
      FUN_0005d65a();
      FUN_0005d5fc();
      FUN_0005d5a2();
      uVar3 = FUN_0005d57e();
      if (*(short *)(extraout_A0_01 + 0x18) == 3) {
        puVar2 = &DAT_00ff0500;
        sVar1 = 5;
        while ((((puVar2[0x10] & 1) == 0 || ((puVar2[0x10] & 0x10) != 0)) ||
               (((puVar2[0x10] & 0x40) != 0 || (puVar2[0x4c] == '\0'))))) {
          puVar2 = puVar2 + 0x100;
          sVar1 = sVar1 + -1;
          if (sVar1 == -1) {
            sVar1 = *(short *)(extraout_A0_01 + 0x24) + -1;
            *(short *)(extraout_A0_01 + 0x24) = sVar1;
            if (sVar1 < 0) {
              _DAT_00ffdd2c = &UNK_0005bac4;
              uVar3 = FUN_0001e2e0();
              return uVar3;
            }
            *(undefined1 *)(extraout_A0_01 + 0x9d) = 1;
            DAT_00ff0477 = 0x20;
            DAT_00ff047c = 0x20;
            DAT_00ff0480 = 0x20;
            uVar3 = FUN_0005d3e6();
            *(undefined1 *)(extraout_A1 + 0x4c) = 4;
            *(undefined1 *)(extraout_A1 + 0x9c) = 4;
            *(undefined1 *)(extraout_A1 + 0x10) = 0x21;
            return uVar3;
          }
        }
        uVar3 = FUN_0005d3e6();
        return uVar3;
      }
    }
    else if (DAT_00fff13e._2_2_ == 0) {
      DAT_00fff142._0_2_ = DAT_00fff142._0_2_ + -1;
      if (-1 < DAT_00fff142._0_2_) {
        FUN_0005d416();
        return CONCAT44(in_D0,in_D1);
      }
      uVar3 = FUN_0005ba66();
      *(undefined2 *)(extraout_A0 + 0x22) = 1;
      *(undefined2 *)(extraout_A0 + 0x24) = 0x100;
    }
    else {
      if (((short)DAT_00fff122 < 0) || (DAT_00fff122 < 0x248)) {
        DAT_00fff122 = DAT_00fff122 + 8;
      }
      project_entity_screen_position();
      FUN_0005d55c();
      FUN_0005d5fc();
      FUN_0005d5a2();
      uVar3 = FUN_0005d57e();
      sVar1 = *(short *)(extraout_A0_00 + 0x24) + -1;
      *(short *)(extraout_A0_00 + 0x24) = sVar1;
      if (sVar1 < 0) {
        *(undefined2 *)(extraout_A0_00 + 0x22) = 2;
        *(undefined2 *)(extraout_A0_00 + 0x24) = 0x28;
        return uVar3;
      }
    }
  }
  return uVar3;
}


