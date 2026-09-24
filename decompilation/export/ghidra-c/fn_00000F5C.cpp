// Ghidra headless export
// Entry: 0x00000F5C
// Name: FUN_00000f5c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F98 FUN_00000f98


undefined8 FUN_00000f5c(void)

{
  uint in_D0;
  short sVar1;
  undefined4 in_D1;
  undefined4 extraout_D1;
  ushort unaff_D2w;
  undefined4 *extraout_A0;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 *extraout_A1;
  undefined2 uStack_8;
  undefined2 uStack_6;
  
  if (((in_D0 ^ in_D0 + unaff_D2w) & 0x10000) == 0) {
    FUN_00000e9c();
    sVar1 = (unaff_D2w >> 6) - 1;
    puVar3 = extraout_A0;
    if (-1 < sVar1) {
      do {
        *extraout_A1 = *puVar3;
        *extraout_A1 = puVar3[1];
        *extraout_A1 = puVar3[2];
        *extraout_A1 = puVar3[3];
        *extraout_A1 = puVar3[4];
        *extraout_A1 = puVar3[5];
        *extraout_A1 = puVar3[6];
        *extraout_A1 = puVar3[7];
        *extraout_A1 = puVar3[8];
        *extraout_A1 = puVar3[9];
        *extraout_A1 = puVar3[10];
        *extraout_A1 = puVar3[0xb];
        *extraout_A1 = puVar3[0xc];
        *extraout_A1 = puVar3[0xd];
        puVar2 = puVar3 + 0xf;
        *extraout_A1 = puVar3[0xe];
        puVar3 = puVar3 + 0x10;
        *extraout_A1 = *puVar2;
        sVar1 = sVar1 + -1;
      } while (sVar1 != -1);
    }
    sVar1 = (unaff_D2w >> 2 & 0xf) - 1;
    puVar2 = puVar3;
    if (-1 < sVar1) {
      do {
        puVar3 = puVar2 + 1;
        *extraout_A1 = *puVar2;
        sVar1 = sVar1 + -1;
        puVar2 = puVar3;
      } while (sVar1 != -1);
    }
    if ((unaff_D2w & 2) != 0) {
      *(undefined2 *)extraout_A1 = *(undefined2 *)puVar3;
    }
    return CONCAT44(in_D0,extraout_D1);
  }
  uStack_8 = (undefined2)((uint)in_D1 >> 0x10);
  uStack_6 = (undefined2)in_D1;
  FUN_00000f98();
  FUN_00000f98();
  return CONCAT62(CONCAT42(in_D0,uStack_8),uStack_6);
}


