// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00015830


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00015830(void)

{
  undefined2 uVar1;
  short sVar2;
  undefined2 extraout_D1w;
  short extraout_D1w_00;
  int in_A0;
  undefined4 extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A1;
  int extraout_A1_00;
  int iVar3;
  undefined1 uVar4;
  undefined2 in_stack_fffffffa;
  undefined4 uVar5;
  undefined2 uVar6;
  
  if ((*(char *)(in_A0 + 0x57) != '\a') && (*(char *)(in_A0 + 0x57) != '\x0e')) {
    if (*(int *)(in_A0 + 0xa2) != 0) {
      uVar4 = 0;
      FUN_0001dbaa();
      uVar6 = 0x586a;
      uVar1 = thunk_FUN_0001a2cc();
      if (!(bool)uVar4) {
        *(undefined4 *)(extraout_A1 + 0xd8) = extraout_A0;
        uVar5 = CONCAT22(uVar1,in_stack_fffffffa);
        uVar1 = *(undefined2 *)(extraout_A1 + 4);
        FUN_00015d3c(uVar5,CONCAT22(extraout_D1w,uVar6));
        *(undefined2 *)(extraout_A0_00 + 0x2e) = uVar1;
        FUN_0001597e();
        if (*(char *)(extraout_A0_01 + 0x57) == '\x06') {
          _DAT_00ffdff2 = 0;
          _DAT_00ffdff4 = (short)*(char *)(extraout_A0_01 + 0x9d);
          iVar3 = _DAT_00ffdffa;
        }
        else {
          iVar3 = extraout_A1_00;
          if (*(char *)(extraout_A0_01 + 0x57) == '\r') {
            _DAT_00ffdff6 = 0;
            _DAT_00ffdff8 = (short)*(char *)(extraout_A0_01 + 0x9d);
            iVar3 = _DAT_00ffdffa;
          }
        }
        *(undefined1 *)(iVar3 + 0xb2) = *(undefined1 *)(extraout_A0_01 + 0x57);
        *(short *)(iVar3 + 0x32) = (short)((uint)uVar5 >> 0x10);
        *(short *)(iVar3 + 0x2a) = (short)uVar5;
        *(undefined1 *)(iVar3 + 2) = 5;
        *(undefined1 *)(iVar3 + 3) = 0;
        sVar2 = (short)*(char *)(extraout_A0_01 + 0x57);
        if (*(short *)(&DAT_00004946 + (short)(sVar2 << 1)) != 0) {
          FUN_000530de(extraout_A0_01,iVar3);
          sVar2 = extraout_D1w_00;
        }
        (**(code **)((int)&PTR_LAB_00004964 + (int)(short)(sVar2 << 2)))();
      }
    }
    return;
  }
  FUN_000049a2();
  return;
}


