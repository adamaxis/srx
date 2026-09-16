// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0000BADE


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000bade(void)

{
  short sVar1;
  undefined2 unaff_D5w;
  undefined2 unaff_D6w;
  int in_A0;
  int extraout_A0;
  int iVar2;
  int extraout_A0_00;
  undefined2 *extraout_A0_01;
  undefined2 *extraout_A0_02;
  undefined2 *extraout_A0_03;
  undefined2 *puVar3;
  int in_A1;
  bool bVar4;
  
  *(undefined2 *)(in_A0 + 0xca) = *(undefined2 *)(in_A1 + 4);
  sVar1 = FUN_0000b77e();
  iVar2 = extraout_A0;
  if ((sVar1 == 0) ||
     ((sVar1 < 0 &&
      (FUN_0000b7b8(), iVar2 = extraout_A0_00, (*(byte *)(extraout_A0_00 + 0xb0) & 1) == 0)))) {
    *(undefined2 *)(iVar2 + 0xb8) = 0x1000;
    FUN_0000bbe4();
    return;
  }
  bVar4 = false;
  if ((*(ushort *)(iVar2 + 0x2e) & 0x3fff) == 0) {
    FUN_0000bb86();
    puVar3 = extraout_A0_03;
    if (!bVar4) {
      *(byte *)(extraout_A0_03 + 0x58) = *(byte *)(extraout_A0_03 + 0x58) & 0xfe;
      *extraout_A0_03 = unaff_D5w;
      extraout_A0_03[2] = unaff_D6w;
      return;
    }
LAB_0000bb66:
    puVar3[0x5c] = 0xffff;
    FUN_0000b442();
    return;
  }
  _DAT_00ffdd3c = *(ushort *)(iVar2 + 0x2e) >> 0xd & 6;
  *(ushort *)(iVar2 + 0xb8) = _DAT_00ffdd3c;
  sVar1 = FUN_0000b732();
  puVar3 = extraout_A0_01;
  if ((sVar1 != 0) && (-1 < sVar1)) {
    extraout_A0_01[0x5c] = _DAT_00ffdd3c;
    sVar1 = FUN_0000b732();
    puVar3 = extraout_A0_02;
    if ((sVar1 != 0) && (-1 < sVar1)) goto LAB_0000bb66;
  }
  puVar3[0x1b] = _DAT_00ffdf66 << 0xc | _DAT_00ffdf66 >> 4;
  *(byte *)((int)puVar3 + 0x11) = *(byte *)((int)puVar3 + 0x11) | 4;
  FUN_0000bbe4();
  return;
}


