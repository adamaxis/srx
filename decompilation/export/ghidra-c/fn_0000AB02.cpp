// Ghidra headless export
// Entry: 0x0000AB02
// Name: FUN_0000ab02
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00016B98 FUN_00016b98
//   0x0001DBF0 reset_entity_service_records
//   0x0001E2EC begin_world_activity


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000ab02(void)

{
  short sVar1;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  undefined *puVar2;
  
  if ((*(byte *)(in_A0 + 0xb0) & 0x80) != 0) {
    _DAT_00ffdd2c = &DAT_0001f210;
    begin_world_activity();
  }
  if (((((*(byte *)(in_A0 + 0x10) & 8) == 0) &&
       (FUN_00016b98(), in_A0 = extraout_A0, (*(byte *)(extraout_A0 + 0x9e) & 0x40) != 0)) &&
      (DAT_00ffe177 = DAT_00ffe177 + -1, DAT_00ffe177 == '\0')) &&
     (reset_entity_service_records(), in_A0 = extraout_A0_00, DAT_00ffe12e != (undefined *)0x0)) {
    _DAT_00ffdd2c = DAT_00ffe12e;
    begin_world_activity();
    DAT_00ffe12e = (undefined *)0x0;
    in_A0 = extraout_A0_01;
  }
  *(byte *)(in_A0 + 0x10) = *(byte *)(in_A0 + 0x10) | 0x40;
  if ((*(byte *)(in_A0 + 0x10) & 0x20) != 0) {
    puVar2 = &DAT_00ff0400;
    sVar1 = 6;
    do {
      if (((puVar2[0x10] & 1) != 0) && (in_A0 == *(int *)(puVar2 + 0xa8))) {
        puVar2[0x10] = puVar2[0x10] | 0x20;
      }
      puVar2 = puVar2 + 0x100;
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
  }
  if ((*(byte *)(in_A0 + 0x10) & 8) != 0) {
    if ((((DAT_00ff0110 & 0x40) != 0) && (((DAT_00ff0210 & 1) == 0 || ((DAT_00ff0210 & 0x40) != 0)))
        ) && (((DAT_00ff0310 & 1) == 0 || ((DAT_00ff0310 & 0x40) != 0)))) {
      DAT_00ffe166 = 0x3c;
    }
    return;
  }
  return;
}


