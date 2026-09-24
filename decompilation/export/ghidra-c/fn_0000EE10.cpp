// Ghidra headless export
// Entry: 0x0000EE10
// Name: FUN_0000ee10
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000EE86 FUN_0000ee86


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000ee10(void)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  ushort *puVar4;
  short sVar5;
  ushort uVar6;
  ushort uVar7;
  short sVar8;
  undefined4 *puVar9;
  ushort *puVar10;
  
  if (_DAT_00ffdf8a == 0) {
    return;
  }
  _DAT_00ffdf8a = 0;
  sVar8 = 0x3ff;
  puVar9 = (undefined4 *)&DAT_00ff1700;
  do {
    *puVar9 = 0;
    sVar8 = sVar8 + -1;
    puVar9 = puVar9 + 1;
    puVar4 = DAT_00ffddec;
  } while (sVar8 != -1);
  while( true ) {
    puVar10 = puVar4;
    uVar6 = *puVar10;
    if ((short)uVar6 < 0) break;
    uVar1 = puVar10[1];
    uVar7 = puVar10[2];
    uVar2 = puVar10[3];
    uVar3 = puVar10[4];
    sVar8 = (uVar7 & 0xffe0) * 2 + (uVar6 >> 5);
    sVar5 = sVar8;
    do {
      do {
        (&DAT_00ff1700)[sVar5] = (char)uVar3;
        uVar6 = uVar6 + 0x20;
        sVar5 = sVar5 + 1;
      } while (uVar6 <= uVar1);
      uVar6 = *puVar10;
      sVar8 = sVar8 + 0x40;
      uVar7 = uVar7 + 0x20;
      puVar4 = puVar10 + 5;
      sVar5 = sVar8;
    } while (uVar7 <= uVar2);
  }
  FUN_0000ee86();
  return;
}


