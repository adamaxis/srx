// Ghidra headless export
// Entry: 0x00013C74
// Name: collect_entity_action_records
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void collect_entity_action_records(void)

{
  char cVar1;
  short sVar2;
  short *unaff_A2;
  short *psVar3;
  char *pcVar4;
  char *pcVar5;
  
  psVar3 = unaff_A2 + 1;
  sVar2 = *unaff_A2;
  pcVar4 = (char *)&DAT_00ffdd2a;
  DAT_00ffdd28 = 0;
  do {
    cVar1 = *(char *)psVar3;
    if ((((cVar1 == '\0') || (cVar1 == '\x01')) || (cVar1 == '\a')) || (cVar1 == '\b')) {
      DAT_00ffdd28 = DAT_00ffdd28 + 1;
      pcVar5 = pcVar4 + 1;
      *pcVar4 = cVar1;
      pcVar4 = pcVar4 + 2;
      *pcVar5 = *(char *)((int)psVar3 + 1);
    }
    else if (cVar1 == '\v') {
      DAT_00ffdd66._0_1_ = *(char *)((int)psVar3 + 1);
    }
    psVar3 = psVar3 + 1;
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  pcVar4[0] = -1;
  pcVar4[1] = -1;
  return;
}


