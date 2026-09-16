// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00004B6C


void FUN_00004b6c(void)

{
  short sVar1;
  short sVar2;
  int in_A0;
  int extraout_A0;
  undefined1 *puVar3;
  short *psVar4;
  
  *(undefined2 *)(in_A0 + 2) = 0;
  FUN_0001a304();
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


