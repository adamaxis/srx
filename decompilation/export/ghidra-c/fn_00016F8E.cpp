// Ghidra headless export
// Entry: 0x00016F8E
// Name: copy_terrain_work_region_rows
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00016FA8 copy_terrain_work_region


void copy_terrain_work_region_rows(void)

{
  int iVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  short sVar5;
  short sVar6;
  short *in_A1;
  short *extraout_A1;
  
  sVar2 = *in_A1;
  sVar3 = in_A1[1];
  sVar4 = in_A1[2];
  if (sVar4 < 0) {
    copy_terrain_work_region();
    sVar2 = *extraout_A1;
    sVar3 = extraout_A1[1];
    sVar4 = extraout_A1[2];
  }
  iVar1 = DAT_00ffdde2;
  sVar3 = sVar3 + -1;
  sVar6 = sVar2 + -1;
  sVar5 = sVar4;
  do {
    do {
      *(undefined2 *)((int)&DAT_00ff4700 + (int)sVar4) = *(undefined2 *)(iVar1 + sVar4);
      sVar4 = sVar4 + 2;
      sVar6 = sVar6 + -1;
    } while (sVar6 != -1);
    sVar4 = DAT_00ffddea + sVar5;
    sVar3 = sVar3 + -1;
    sVar6 = sVar2 + -1;
    sVar5 = sVar4;
  } while (sVar3 != -1);
  return;
}


