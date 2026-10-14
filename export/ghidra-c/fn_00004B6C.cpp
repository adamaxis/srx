// Ghidra headless export
// Entry: 0x00004B6C
// Name: reset_effect_record
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001A304 clear_effect_slot


void reset_effect_record(void)

{
  short sVar1;
  short sVar2;
  int in_A0;
  int extraout_A0;
  undefined1 *puVar3;
  short *psVar4;
  
  *(undefined2 *)(in_A0 + 2) = 0;
  clear_effect_slot();
  sVar1 = *(short *)(extraout_A0 + 0x34);
  sVar2 = 4;
  puVar3 = (undefined1 *)(extraout_A0 + 0x36);
  psVar4 = *(short **)((int)&PTR_DAT_00004bb0 + (int)(short)(*(short *)(extraout_A0 + 6) << 2));
  do {
    (&DAT_00ff1700)[(short)(*psVar4 + sVar1)] = *puVar3;
    sVar2 = sVar2 + -1;
    puVar3 = puVar3 + 1;
    psVar4 = psVar4 + 1;
  } while (sVar2 != -1);
  return;
}


