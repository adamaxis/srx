// Ghidra headless export
// Entry: 0x00002F32
// Name: FUN_00002f32
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00002BD2 FUN_00002bd2
//   0x0000301E FUN_0000301e
//   0x00003032 scene_uses_alternate_profile
//   0x0000C7F8 FUN_0000c7f8


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_00002f32(void)

{
  ushort *puVar1;
  uint uVar2;
  short sVar3;
  ushort *extraout_A1;
  undefined2 *puVar4;
  undefined2 *puVar5;
  undefined2 *puVar6;
  
  FUN_0000c7f8();
  DAT_00ffdf40 = 0;
  _DAT_00ffdf8a = 0xffff;
  sVar3 = 0x30;
  puVar4 = &DAT_00ffddc4;
  puVar5 = &DAT_00ffde26;
  do {
    *puVar4 = *puVar5;
    sVar3 = sVar3 + -1;
    puVar4 = puVar4 + 1;
    puVar5 = puVar5 + 1;
  } while (sVar3 != -1);
  DAT_00fffbf8 = (undefined1)DAT_00ffddc4;
  scene_uses_alternate_profile();
  DAT_00ffddc6 = DAT_00ffdf50;
  DAT_00ffddc8 = DAT_00ffdf52;
  FUN_00002bd2();
  FUN_0000301e();
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
    FUN_00002ff0();
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


