// Ghidra headless export
// Entry: 0x00014492
// Name: FUN_00014492
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound
//   0x0001DBF0 FUN_0001dbf0


void FUN_00014492(void)

{
  int iVar1;
  int iVar2;
  short sVar3;
  int extraout_A0;
  int extraout_A0_00;
  int in_A0;
  int extraout_A0_01;
  int *piVar4;
  
  if ((*(byte *)(in_A0 + 0x10) & 8) != 0) {
    if (DAT_00ffdf94 == 0) {
      if (DAT_00ffe162._0_1_ == '\0') {
        FUN_0001dbf0();
        in_A0 = extraout_A0;
      }
      *(undefined4 *)(in_A0 + 0xa2) = 0;
      return;
    }
    sVar3 = DAT_00ffdf94 + -1;
    piVar4 = &DAT_00ffdf96;
    do {
      if (*(int *)(in_A0 + 0xa2) == *piVar4) {
        return;
      }
      sVar3 = sVar3 + -1;
      piVar4 = piVar4 + 1;
    } while (sVar3 != -1);
    sVar3 = sample_source_random_bound();
    *(undefined4 *)(extraout_A0_00 + 0xa2) =
         *(undefined4 *)((int)&DAT_00ffdf96 + (int)(short)(sVar3 << 2));
    return;
  }
  iVar1 = *(int *)(in_A0 + 0xa2);
  if (DAT_00ffdfc8 != 0) {
    sVar3 = DAT_00ffdfc8 + -1;
    piVar4 = &DAT_00ffdfca;
    do {
      if (iVar1 == *piVar4) {
        return;
      }
      sVar3 = sVar3 + -1;
      piVar4 = piVar4 + 1;
    } while (sVar3 != -1);
    sVar3 = sample_source_random_bound();
    iVar2 = *(int *)((int)&DAT_00ffdfca + (int)(short)(sVar3 << 2));
    *(int *)(extraout_A0_01 + 0xa2) = iVar2;
    if (iVar2 != iVar1) {
      *(undefined1 *)(extraout_A0_01 + 0xb5) = 6;
    }
    return;
  }
  *(undefined4 *)(in_A0 + 0xa2) = 0;
  return;
}


