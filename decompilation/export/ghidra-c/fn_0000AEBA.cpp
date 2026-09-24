// Ghidra headless export
// Entry: 0x0000AEBA
// Name: FUN_0000aeba
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000aeba(void)

{
  uint uVar1;
  uint uVar2;
  short sVar3;
  undefined4 uVar4;
  short sVar5;
  
  DAT_00ffdfec = DAT_00ffdfec + 8;
  if (DAT_00ffdfec < 0xa0) {
    uVar1 = (uint)DAT_00ffdfee;
    uVar2 = (uint)DAT_00ffdff0;
    uVar4 = *(undefined4 *)(&DAT_0000af3e + (short)((DAT_00ffdf88 >> 2 & 3) << 2));
    sVar3 = 0x1f;
    sVar5 = 0;
    do {
      build_title_sprite_records(uVar1,uVar2,sVar5,uVar4);
      sVar5 = sVar5 + 8;
      sVar3 = sVar3 + -1;
    } while (sVar3 != -1);
    return;
  }
  _DAT_00ffdfea = 0;
  return;
}


