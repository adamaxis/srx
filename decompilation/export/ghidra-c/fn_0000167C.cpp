// Ghidra headless export
// Entry: 0x0000167C
// Name: decode_source_coordinate_stream
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined8 decode_source_coordinate_stream(void)

{
  short sVar1;
  bool bVar2;
  byte bVar3;
  undefined4 in_D0;
  byte bVar4;
  undefined4 in_D1;
  short sVar5;
  short sVar6;
  short *in_A0;
  byte *pbVar7;
  byte *pbVar8;
  short *in_A1;
  short *psVar9;
  
  pbVar8 = (byte *)(in_A0 + 1);
  sVar1 = *in_A0;
  while( true ) {
    pbVar7 = pbVar8 + 1;
    bVar4 = *pbVar8;
    if (bVar4 == 0) break;
    if (bVar4 < 0xc1) {
      sVar6 = 1;
      if ((bVar4 < 0x81) && (sVar6 = 0, bVar4 < 0x41)) {
        sVar6 = -1;
      }
      if ((bVar4 & 0x20) == 0) {
        pbVar8 = pbVar8 + 2;
        sVar5 = sVar1 + (ushort)*pbVar7;
        psVar9 = in_A1;
        bVar4 = bVar4 & 0x1f;
        do {
          in_A1 = psVar9 + 1;
          *psVar9 = sVar5;
          sVar5 = sVar6 + sVar5;
          bVar2 = '\0' < (char)bVar4;
          psVar9 = in_A1;
          bVar4 = bVar4 - 1;
        } while (bVar2);
      }
      else {
        pbVar8 = pbVar8 + 3;
        sVar5 = sVar1 + *(short *)pbVar7;
        psVar9 = in_A1;
        bVar4 = bVar4 & 0x1f;
        do {
          in_A1 = psVar9 + 1;
          *psVar9 = sVar5;
          sVar5 = sVar6 + sVar5;
          bVar2 = '\0' < (char)bVar4;
          psVar9 = in_A1;
          bVar4 = bVar4 - 1;
        } while (bVar2);
      }
    }
    else if ((bVar4 & 0x20) == 0) {
      psVar9 = in_A1;
      bVar4 = bVar4 & 0x1f;
      do {
        pbVar8 = pbVar7 + 1;
        in_A1 = psVar9 + 1;
        *psVar9 = sVar1 + (ushort)*pbVar7;
        bVar3 = bVar4 - 1;
        bVar2 = '\0' < (char)bVar4;
        pbVar7 = pbVar8;
        psVar9 = in_A1;
        bVar4 = bVar3;
      } while (bVar3 != 0 && bVar2);
    }
    else {
      psVar9 = in_A1;
      bVar4 = bVar4 & 0x1f;
      do {
        pbVar8 = pbVar7 + 2;
        in_A1 = psVar9 + 1;
        *psVar9 = sVar1 + *(short *)pbVar7;
        bVar3 = bVar4 - 1;
        bVar2 = '\0' < (char)bVar4;
        pbVar7 = pbVar8;
        psVar9 = in_A1;
        bVar4 = bVar3;
      } while (bVar3 != 0 && bVar2);
    }
  }
  return CONCAT44(in_D0,in_D1);
}


