// Ghidra headless export
// Entry: 0x00015554
// Name: FUN_00015554
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001562E service_entity_auxiliary_behavior


void FUN_00015554(void)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  short sVar4;
  ushort *in_A0;
  ushort *puVar5;
  ushort *extraout_A1;
  ushort *extraout_A1_00;
  
  uVar1 = *in_A0;
  uVar2 = in_A0[2];
  puVar5 = &DAT_00ff0100;
  sVar4 = 9;
  if (*(char *)((int)in_A0 + 0x57) != '\x0e') {
    do {
      if (((puVar5[8] & 0x100) != 0) && ((puVar5[8] & 0x4000) == 0)) {
        uVar3 = *puVar5 - 0x280;
        if (*puVar5 < 0x280) {
          uVar3 = 0;
        }
        if ((uVar3 <= uVar1) && (uVar1 < (ushort)(uVar3 + 0x500))) {
          uVar3 = puVar5[2] - 0x280;
          if (puVar5[2] < 0x280) {
            uVar3 = 0;
          }
          if ((uVar3 <= uVar2) && (uVar2 < (ushort)(uVar3 + 0x500))) {
            service_entity_auxiliary_behavior();
            puVar5 = extraout_A1;
          }
        }
      }
      puVar5 = puVar5 + 0x80;
      sVar4 = sVar4 + -1;
    } while (sVar4 != -1);
    return;
  }
  do {
    if (((puVar5[8] & 0x100) != 0) && ((puVar5[8] & 0x4000) == 0)) {
      uVar3 = *puVar5 - 0x140;
      if (*puVar5 < 0x140) {
        uVar3 = 0;
      }
      if ((uVar3 <= uVar1) && (uVar1 < (ushort)(uVar3 + 0x280))) {
        uVar3 = puVar5[2] - 0x140;
        if (puVar5[2] < 0x140) {
          uVar3 = 0;
        }
        if ((uVar3 <= uVar2) && (uVar2 < (ushort)(uVar3 + 0x280))) {
          service_entity_auxiliary_behavior();
          puVar5 = extraout_A1_00;
        }
      }
    }
    puVar5 = puVar5 + 0x80;
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
  return;
}


