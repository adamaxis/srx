// Ghidra headless export
// Entry: 0x0001B45C
// Name: convert_world_routine_0x0001b45c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D44 advance_source_prng
//   0x00000E18 write_word_stream
//   0x00019F78 finalize_menu_slot_code_six
//   0x0001B5A6 reset_word_stream_area


void convert_world_routine_0x0001b45c(void)

{
  int iVar1;
  undefined2 uVar2;
  undefined2 uVar3;
  short sVar4;
  uint uVar5;
  short sVar6;
  short sVar7;
  short sVar8;
  int in_A0;
  short *extraout_A1;
  short *extraout_A1_00;
  short *psVar9;
  
  DAT_00ffe0e8 = DAT_00ffe0e8 + -1;
  if (DAT_00ffe0e8 < 0) {
    DAT_00ffe0e8 = DAT_00ffe0e6;
    DAT_00ffe16a = DAT_00ffe16a + -1;
    if (DAT_00ffe16a == '\0') {
      *(byte *)(in_A0 + 0x12) = *(byte *)(in_A0 + 0x12) & 0xfb;
      finalize_menu_slot_code_six();
      reset_word_stream_area();
      return;
    }
  }
  sVar7 = 0xaf;
  uVar2 = advance_source_prng();
  uVar3 = advance_source_prng();
  uVar5 = CONCAT22(uVar2,uVar3);
  sVar6 = *(short *)(&DAT_0001b592 + (short)((short)(char)(DAT_00ffe16a + '\x01') << 1));
  psVar9 = extraout_A1;
  sVar8 = 0;
  do {
    iVar1 = (int)uVar5 >> 0x1f;
    uVar5 = uVar5 << 1 | -iVar1;
    if (-iVar1 == 0) {
      sVar4 = *psVar9 + 1;
    }
    else {
      sVar4 = *psVar9 + -1;
    }
    sVar8 = sVar8 - sVar4;
    if (sVar8 < 0) {
      if (sVar6 <= -sVar8) {
        sVar4 = sVar4 + -1;
      }
    }
    else if (sVar6 <= sVar8) {
      sVar4 = sVar4 + 1;
    }
    *psVar9 = sVar4;
    psVar9 = psVar9 + 2;
    sVar7 = sVar7 + -1;
    sVar8 = sVar4;
  } while (sVar7 != -1);
  sVar8 = 200;
  uVar2 = advance_source_prng();
  uVar3 = advance_source_prng();
  uVar5 = CONCAT22(uVar2,uVar3);
  sVar6 = (short)DAT_00ffe16a;
  if (2 < sVar6) {
    sVar6 = 2;
  }
  sVar6 = *(short *)(&DAT_0001b592 + (short)(sVar6 << 1));
  psVar9 = extraout_A1_00;
  sVar7 = 0;
  do {
    iVar1 = (int)uVar5 >> 0x1f;
    uVar5 = uVar5 << 1 | -iVar1;
    if (-iVar1 == 0) {
      sVar4 = *psVar9 + 1;
    }
    else {
      sVar4 = *psVar9 + -1;
    }
    sVar7 = sVar7 - sVar4;
    if (sVar7 < 0) {
      if (sVar6 <= -sVar7) {
        sVar4 = sVar4 + -1;
      }
    }
    else if (sVar6 <= sVar7) {
      sVar4 = sVar4 + 1;
    }
    *psVar9 = sVar4;
    psVar9 = psVar9 + 2;
    sVar8 = sVar8 + -1;
    sVar7 = sVar4;
  } while (sVar8 != -1);
  write_word_stream();
  DAT_00ffef14 = DAT_00ffef14 | 0xc;
  return;
}


