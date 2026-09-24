// Ghidra headless export
// Entry: 0x00005B1E
// Name: sort_entity_slots_by_depth
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void sort_entity_slots_by_depth(void)

{
  bool bVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  int iVar5;
  int iVar6;
  int *piVar7;
  
  do {
    bVar1 = false;
    sVar4 = 0xe;
    iVar5 = DAT_00ffde8a;
    piVar7 = &DAT_00ffde8e;
    do {
      iVar6 = *piVar7;
      sVar2 = *(short *)(iVar5 + 4);
      if (((*(byte *)(iVar5 + 0x10) & 0x10) != 0) || ((*(byte *)(iVar5 + 0x10) & 0x40) != 0)) {
        sVar2 = 0;
      }
      sVar3 = *(short *)(iVar6 + 4);
      if (((*(byte *)(iVar6 + 0x10) & 0x10) != 0) || ((*(byte *)(iVar6 + 0x10) & 0x40) != 0)) {
        sVar3 = 0;
      }
      if (sVar2 < sVar3) {
        piVar7[-1] = iVar6;
        *piVar7 = iVar5;
        bVar1 = true;
        iVar6 = iVar5;
      }
      sVar4 = sVar4 + -1;
      iVar5 = iVar6;
      piVar7 = piVar7 + 1;
    } while (sVar4 != -1);
  } while (bVar1);
  return;
}


