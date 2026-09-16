// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00026696


void FUN_00026696(void)

{
  ushort uVar1;
  short sVar2;
  short sVar3;
  short sVar4;
  short *psVar5;
  short *psVar6;
  undefined4 *puVar7;
  undefined4 *puVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  undefined4 *unaff_A5;
  ushort *unaff_A6;
  
  psVar5 = (short *)*unaff_A5;
  puVar7 = (undefined4 *)&DAT_00ff8700;
  uVar1 = *unaff_A6;
  sVar2 = (unaff_A6[1] >> 3) - 1;
  do {
    sVar4 = (uVar1 >> 3) - 1;
    psVar6 = psVar5;
    puVar8 = puVar7;
    do {
      psVar5 = psVar6 + 1;
      sVar3 = 7;
      puVar9 = puVar8;
      puVar10 = (undefined4 *)((uint)(ushort)(*psVar6 << 5) + unaff_A5[1]);
      do {
        *puVar9 = *puVar10;
        puVar9 = (undefined4 *)((uint)(ushort)((uVar1 >> 3) << 2) + (int)puVar9);
        sVar3 = sVar3 + -1;
        puVar10 = puVar10 + 1;
      } while (sVar3 != -1);
      puVar8 = puVar8 + 1;
      sVar4 = sVar4 + -1;
      psVar6 = psVar5;
    } while (sVar4 != -1);
    puVar7 = (undefined4 *)((uint)(ushort)(*unaff_A6 << 2) + (int)puVar7);
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return;
}


