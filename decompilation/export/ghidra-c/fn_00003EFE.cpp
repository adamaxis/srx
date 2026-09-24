// Ghidra headless export
// Entry: 0x00003EFE
// Name: FUN_00003efe
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003F9E calculate_field_tile_source_offset


undefined8 FUN_00003efe(void)

{
  byte bVar1;
  short sVar2;
  ushort uVar3;
  undefined4 in_D0;
  uint uVar4;
  undefined4 in_D1;
  byte bVar5;
  ushort uVar6;
  ushort uVar7;
  short sVar8;
  int in_A0;
  byte *extraout_A1;
  byte *pbVar9;
  int unaff_A2;
  byte *pbVar10;
  byte *pbVar11;
  byte *pbVar12;
  short sVar14;
  byte *pbVar13;
  
  uVar4 = (uint)(ushort)((DAT_00ffddc6 >> 4) +
                        *(short *)(in_A0 + 10) + *(short *)(unaff_A2 + 8) + -0x80);
  sVar2 = (DAT_00ffddc8 >> 4) + *(short *)(in_A0 + 0xc) + *(short *)(unaff_A2 + 10) + -0x80;
  uVar6 = *(ushort *)(unaff_A2 + 0xc) >> 2;
  uVar3 = *(ushort *)(unaff_A2 + 0xc) & 3;
  pbVar12 = *(byte **)(unaff_A2 + 2);
  pbVar11 = *(byte **)(in_A0 + 0x44);
  uVar7 = uVar3;
  sVar14 = sVar2;
  do {
    do {
      calculate_field_tile_source_offset(uVar4,sVar14);
      sVar8 = 0x1f;
      pbVar9 = extraout_A1;
      pbVar10 = pbVar11;
      pbVar13 = pbVar12;
      do {
        pbVar12 = pbVar13 + 1;
        bVar1 = *pbVar13;
        bVar5 = bVar1;
        if (bVar1 != 0) {
          if ((bVar1 & 0xf0) == 0) {
            bVar5 = 0xf;
          }
          else {
            bVar5 = 0xf0;
            if ((bVar1 & 0xf) != 0) {
              bVar5 = 0xff;
            }
          }
        }
        pbVar11 = pbVar10 + 1;
        *pbVar10 = *pbVar9 & bVar5;
        sVar8 = sVar8 + -1;
        pbVar9 = pbVar9 + 1;
        pbVar10 = pbVar11;
        pbVar13 = pbVar12;
      } while (sVar8 != -1);
      uVar7 = uVar7 - 1;
      sVar14 = sVar14 + 8;
    } while (uVar7 != 0xffff);
    uVar4 = (uint)(ushort)((short)uVar4 + 8);
    uVar6 = uVar6 - 1;
    uVar7 = uVar3;
    sVar14 = sVar2;
  } while (uVar6 != 0xffff);
  return CONCAT44(in_D0,in_D1);
}


