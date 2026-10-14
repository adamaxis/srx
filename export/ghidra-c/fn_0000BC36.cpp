// Ghidra headless export
// Entry: 0x0000BC36
// Name: build_collision_probe_points
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void build_collision_probe_points(void)

{
  short sVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  short sVar5;
  short *in_A0;
  short *psVar6;
  short *psVar7;
  
  if (DAT_00ffdf40 != 0) {
    return;
  }
  psVar6 = &DAT_00ffdf44;
  sVar4 = 0;
  sVar1 = *in_A0;
  sVar2 = in_A0[2];
  sVar5 = 2;
  do {
    sVar3 = sVar4 + 2;
    psVar7 = psVar6 + 1;
    *psVar6 = sVar1 + *(short *)(&DAT_0000bd34 + sVar4);
    sVar4 = sVar4 + 4;
    psVar6 = psVar6 + 2;
    *psVar7 = sVar2 + *(short *)(&DAT_0000bd34 + sVar3);
    sVar5 = sVar5 + -1;
  } while (sVar5 != -1);
  DAT_00ffdf50 = DAT_00ffddc6;
  DAT_00ffdf52 = DAT_00ffddc8;
  return;
}


