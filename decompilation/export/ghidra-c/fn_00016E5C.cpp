// Ghidra headless export
// Entry: 0x00016E5C
// Name: process_terrain_work_entries
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00016EA0 fill_terrain_work_region


short process_terrain_work_entries(void)

{
  short *psVar1;
  short *psVar2;
  short *extraout_A1;
  char *pcVar3;
  char *pcVar4;
  
  psVar1 = *(short **)(DAT_00ffde1e + 8);
  pcVar3 = (char *)&DAT_00ffb700;
  while (psVar2 = psVar1, *psVar2 != 0) {
    pcVar4 = pcVar3 + 1;
    if (*pcVar3 == '\0') {
      if (psVar2[2] < 0) {
        pcVar4 = pcVar3;
      }
      fill_terrain_work_region();
      psVar1 = extraout_A1;
      pcVar3 = pcVar4;
    }
    else {
      psVar1 = psVar2 + 3;
      pcVar3 = pcVar4;
      if (psVar2[2] < 0) {
        psVar1 = psVar2 + 6;
      }
    }
  }
  return *psVar2;
}


