// Ghidra headless export
// Entry: 0x00016ECA
// Name: process_terrain_work_batch
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00016F14 mark_terrain_boundary_for_mode
//   0x00016F8E copy_terrain_work_region_rows


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 process_terrain_work_batch(void)

{
  short *psVar1;
  undefined2 uVar2;
  short *psVar3;
  short *extraout_A1;
  char *pcVar4;
  char *pcVar5;
  bool bVar6;
  
  psVar1 = *(short **)(DAT_00ffde1e + 8);
  pcVar4 = (char *)&DAT_00ffb700;
  do {
    while( true ) {
      psVar3 = psVar1;
      if (*psVar3 == 0) {
        return 0;
      }
      pcVar5 = pcVar4 + 1;
      bVar6 = false;
      if (*pcVar4 == '\0') break;
      psVar1 = psVar3 + 3;
      pcVar4 = pcVar5;
      if (psVar3[2] < 0) {
        psVar1 = psVar3 + 6;
      }
    }
    mark_terrain_boundary_for_mode();
    psVar1 = extraout_A1;
    pcVar4 = pcVar5;
  } while (!bVar6);
  uVar2 = copy_terrain_work_region_rows();
  _DAT_00ffe102 = 1;
  return uVar2;
}


