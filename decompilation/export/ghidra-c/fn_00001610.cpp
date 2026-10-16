// Ghidra headless export
// Entry: 0x00001610
// Name: decode_source_byte_stream
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined8 decode_source_byte_stream(void)

{
  short sVar1;
  byte bVar2;
  byte in_D0b;
  undefined4 in_D1;
  short sVar3;
  short sVar4;
  short sVar5;
  short sVar6;
  byte *in_A1;
  byte *pbVar7;
  short *unaff_A3;
  byte *pbVar8;
  byte *pbVar9;
  bool bVar10;
  
  pbVar9 = (byte *)(unaff_A3 + 1);
  sVar1 = *unaff_A3;
  sVar3 = 1;
  sVar4 = sVar1;
  do {
    while( true ) {
      sVar3 = sVar3 + -1;
      pbVar8 = pbVar9;
      if (sVar3 == 0) {
        pbVar8 = pbVar9 + 1;
        in_D0b = *pbVar9;
        sVar3 = 8;
      }
      bVar10 = CARRY1(in_D0b,in_D0b);
      in_D0b = in_D0b * '\x02';
      if (bVar10) break;
      pbVar9 = pbVar8 + 1;
      *in_A1 = *pbVar8;
      in_A1 = in_A1 + 1;
joined_r0x0000166c:
      sVar4 = sVar4 + -1;
      if (sVar4 == -1) goto LAB_00001670;
    }
    bVar2 = *pbVar8;
    if ((char)bVar2 < '\0') {
      pbVar9 = pbVar8 + 2;
      sVar6 = (ushort)pbVar8[1] << 3;
      sVar5 = (bVar2 & 0xf) + 2;
      sVar4 = sVar4 - sVar5;
      pbVar8 = in_A1;
      pbVar7 = in_A1 + -(int)CONCAT11((char)((ushort)sVar6 >> 8),bVar2 >> 4 & 7 | (byte)sVar6);
      do {
        in_A1 = pbVar8 + 1;
        *pbVar8 = *pbVar7;
        sVar5 = sVar5 + -1;
        pbVar8 = in_A1;
        pbVar7 = pbVar7 + 1;
      } while (sVar5 != -1);
      goto joined_r0x0000166c;
    }
    sVar5 = (bVar2 & 0x7f) + 2;
    sVar4 = sVar4 - sVar5;
    pbVar9 = pbVar8 + 2;
    bVar2 = pbVar8[1];
    pbVar8 = in_A1;
    do {
      in_A1 = pbVar8 + 1;
      *pbVar8 = bVar2;
      sVar5 = sVar5 + -1;
      pbVar8 = in_A1;
    } while (sVar5 != -1);
    sVar4 = sVar4 + -1;
  } while (sVar4 != -1);
LAB_00001670:
  return CONCAT44(CONCAT22(sVar1 >> 0xf,sVar1 + 1),in_D1);
}


