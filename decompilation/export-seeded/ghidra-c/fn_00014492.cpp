// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00014492


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

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
    if (_DAT_00ffdf94 == 0) {
      if (DAT_00ffe162 == '\0') {
        FUN_0001dbf0();
        in_A0 = extraout_A0;
      }
      *(undefined4 *)(in_A0 + 0xa2) = 0;
      return;
    }
    sVar3 = _DAT_00ffdf94 + -1;
    piVar4 = (int *)&DAT_00ffdf96;
    do {
      if (*(int *)(in_A0 + 0xa2) == *piVar4) {
        return;
      }
      sVar3 = sVar3 + -1;
      piVar4 = piVar4 + 1;
    } while (sVar3 != -1);
    sVar3 = FUN_00000d3a();
    *(undefined4 *)(extraout_A0_00 + 0xa2) = *(undefined4 *)(&DAT_00ffdf96 + (short)(sVar3 << 2));
    return;
  }
  iVar1 = *(int *)(in_A0 + 0xa2);
  if (_DAT_00ffdfc8 != 0) {
    sVar3 = _DAT_00ffdfc8 + -1;
    piVar4 = (int *)&DAT_00ffdfca;
    do {
      if (iVar1 == *piVar4) {
        return;
      }
      sVar3 = sVar3 + -1;
      piVar4 = piVar4 + 1;
    } while (sVar3 != -1);
    sVar3 = FUN_00000d3a();
    iVar2 = *(int *)(&DAT_00ffdfca + (short)(sVar3 << 2));
    *(int *)(extraout_A0_01 + 0xa2) = iVar2;
    if (iVar2 != iVar1) {
      *(undefined1 *)(extraout_A0_01 + 0xb5) = 6;
    }
    return;
  }
  *(undefined4 *)(in_A0 + 0xa2) = 0;
  return;
}


