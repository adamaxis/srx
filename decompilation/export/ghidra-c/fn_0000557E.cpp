// Ghidra headless export
// Entry: 0x0000557E
// Name: FUN_0000557e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00005622 clear_entity_runtime_fields
//   0x0000563E FUN_0000563e
//   0x00005662 FUN_00005662
//   0x0000AFF2 prepare_entity_auxiliary_behavior
//   0x0001A304 clear_effect_slot


void FUN_0000557e(void)

{
  short sVar1;
  int extraout_A0;
  undefined4 extraout_A0_00;
  int iVar2;
  
  if (-1 < DAT_00ffdff2) {
    DAT_00ffdff2 = DAT_00ffdff2 + -1;
    if (-1 < DAT_00ffdff2) {
      return;
    }
    DAT_00ffdff4 = DAT_00ffdff4 + -1;
  }
  FUN_00005662();
  if (*(char *)(extraout_A0 + 0xb2) != '\0') {
    return;
  }
  sVar1 = 8;
  iVar2 = extraout_A0;
  do {
    if (8 < *(short *)(iVar2 + 0x13e)) break;
    if (((*(char *)(iVar2 + 0x1de) == '\x06') && ((*(byte *)(iVar2 + 0x110) & 1) != 0)) &&
       ((*(byte *)(iVar2 + 0x110) & 0x40) == 0)) {
      FUN_0000563e();
      prepare_entity_auxiliary_behavior(extraout_A0_00);
      return;
    }
    sVar1 = sVar1 + -1;
    iVar2 = iVar2 + 0x100;
  } while (sVar1 != -1);
  if (-1 < DAT_00ffdff4) {
    DAT_00ffdff2 = 0x1e;
    FUN_0000563e();
    return;
  }
  clear_effect_slot();
  clear_entity_runtime_fields();
  return;
}


