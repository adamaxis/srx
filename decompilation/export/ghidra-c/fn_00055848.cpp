// Ghidra headless export
// Entry: 0x00055848
// Name: convert_intro_routine_0x00055848
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D3A sample_source_random_bound
//   0x00000DBA count_thresholded_random_rolls
//   0x00055740 convert_intro_routine_0x00055740
//   0x00055786 convert_intro_routine_0x00055786
//   0x00055ED6 convert_intro_routine_0x00055ed6
//   0x00055F20 convert_intro_routine_0x00055f20


short convert_intro_routine_0x00055848(void)

{
  byte bVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  
  convert_intro_routine_0x00055ed6();
  convert_intro_routine_0x00055f20();
  convert_intro_routine_0x00055740();
  convert_intro_routine_0x00055786();
  count_thresholded_random_rolls();
  bVar1 = DAT_00fff0dd >> 1;
  sVar2 = sample_source_random_bound();
  sVar3 = (sVar2 + 1) - (ushort)bVar1;
  if (sVar3 != 0 && (short)(ushort)bVar1 <= (short)(sVar2 + 1)) {
    sVar2 = sample_source_random_bound();
    if ((short)(sVar2 + -1) < 0) {
      sVar4 = sVar3 * -0xc;
    }
    else {
      sVar4 = 0;
      if (sVar2 != 1) {
        sVar4 = sVar3 * 0xc;
      }
    }
    sVar2 = sample_source_random_bound();
    sVar3 = sVar2 + -1;
    if (((-1 < sVar3) && (sVar3 == 0 || sVar2 < 1)) && (sVar4 == 0)) {
      sVar3 = sample_source_random_bound();
    }
  }
  return sVar3;
}


