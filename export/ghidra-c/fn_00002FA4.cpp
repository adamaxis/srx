// Ghidra headless export
// Entry: 0x00002FA4
// Name: snapshot_entity_anchors
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00002FF0 find_ready_entity_slot


uint snapshot_entity_anchors(void)

{
  ushort *puVar1;
  uint uVar2;
  short sVar3;
  ushort *extraout_A1;
  undefined2 *puVar4;
  undefined2 *puVar5;
  undefined2 *puVar6;
  
  puVar4 = &DAT_00ff0100;
  puVar5 = &DAT_00ffdf44;
  sVar3 = 2;
  do {
    puVar6 = puVar5 + 1;
    *puVar4 = *puVar5;
    puVar5 = puVar5 + 2;
    puVar4[2] = *puVar6;
    puVar4 = puVar4 + 0x80;
    sVar3 = sVar3 + -1;
  } while (sVar3 != -1);
  uVar2 = DAT_00fffc3c;
  if (DAT_00fffc3c != 0) {
    find_ready_entity_slot();
    uVar2 = (uint)(ushort)(DAT_00ff0100 + 0x80);
    *extraout_A1 = DAT_00ff0100 + 0x80;
    extraout_A1[2] = DAT_00ff0104;
  }
  puVar1 = DAT_00ffe114;
  if (DAT_00ffe114 != (ushort *)0x0) {
    DAT_00ffe114 = (ushort *)0x0;
    DAT_00ffddc6 = *puVar1 & 0xff80;
    DAT_00ffddc8 = puVar1[1] & 0xff80;
    DAT_00ffdf36 = 0xffff;
    DAT_00ffdf38 = 0xffff;
    DAT_00ff0100 = puVar1[2];
    DAT_00ff0104 = puVar1[3];
    DAT_00ff0200 = puVar1[4];
    DAT_00ff0204 = puVar1[5];
    DAT_00ff0300 = puVar1[6];
    DAT_00ff0304 = puVar1[7];
    return uVar2;
  }
  return uVar2;
}


