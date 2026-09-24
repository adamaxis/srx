// Ghidra headless export
// Entry: 0x00001530
// Name: decode_and_upload_source_stream
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C FUN_00000e9c


undefined6 decode_and_upload_source_stream(void)

{
  ushort uVar1;
  byte bVar2;
  undefined1 uVar3;
  ushort uVar4;
  byte bVar5;
  undefined4 in_D1;
  short sVar6;
  ushort uVar7;
  ushort uVar8;
  short sVar9;
  short sVar10;
  ushort uVar11;
  uint unaff_D7;
  int extraout_A1;
  ushort *unaff_A3;
  ushort *puVar12;
  ushort *puVar13;
  bool bVar14;
  
  uVar11 = 0;
  puVar13 = unaff_A3 + 1;
  uVar1 = *unaff_A3;
  VDP_CTRL = 0x8f02;
  bVar5 = FUN_00000e9c();
  sVar6 = 1;
  uVar8 = uVar1;
  do {
    while( true ) {
      sVar6 = sVar6 + -1;
      puVar12 = puVar13;
      if (sVar6 == 0) {
        puVar12 = (ushort *)((int)puVar13 + 1);
        bVar5 = *(byte *)puVar13;
        sVar6 = 8;
      }
      bVar14 = CARRY1(bVar5,bVar5);
      bVar5 = bVar5 * '\x02';
      if (bVar14) break;
      puVar13 = (ushort *)((int)puVar12 + 1);
      unaff_D7 = CONCAT31((int3)(unaff_D7 >> 8),*(byte *)puVar12);
      *(byte *)(extraout_A1 + (short)uVar11) = *(byte *)puVar12;
      uVar11 = uVar11 + 1 & 0x7ff;
      if ((uVar8 & 1) == 0) {
        VDP_DATA = (word)unaff_D7;
      }
      else {
        unaff_D7 = (uint)(ushort)((word)unaff_D7 << 8);
      }
joined_r0x00001600:
      uVar8 = uVar8 - 1;
      if (uVar8 == 0xffff) goto LAB_00001604;
    }
    bVar2 = *(byte *)puVar12;
    if ((char)bVar2 < '\0') {
      puVar13 = puVar12 + 1;
      sVar10 = (ushort)*(byte *)((int)puVar12 + 1) << 3;
      sVar9 = (bVar2 & 0xf) + 2;
      uVar7 = uVar11 - CONCAT11((char)((ushort)sVar10 >> 8),bVar2 >> 4 & 7 | (byte)sVar10);
      uVar4 = uVar8;
      do {
        uVar8 = uVar4;
        uVar3 = *(undefined1 *)(extraout_A1 + (short)(uVar7 & 0x7ff));
        unaff_D7 = CONCAT31((int3)(unaff_D7 >> 8),uVar3);
        uVar7 = (uVar7 & 0x7ff) + 1;
        *(undefined1 *)(extraout_A1 + (short)uVar11) = uVar3;
        uVar11 = uVar11 + 1 & 0x7ff;
        if ((uVar8 & 1) == 0) {
          VDP_DATA = (word)unaff_D7;
        }
        else {
          unaff_D7 = (uint)(ushort)((word)unaff_D7 << 8);
        }
        sVar9 = sVar9 + -1;
        uVar4 = uVar8 - 1;
      } while (sVar9 != -1);
      goto joined_r0x00001600;
    }
    sVar9 = (bVar2 & 0x7f) + 2;
    puVar13 = puVar12 + 1;
    bVar2 = *(byte *)((int)puVar12 + 1);
    do {
      uVar7 = uVar8;
      unaff_D7 = CONCAT31((int3)(unaff_D7 >> 8),bVar2);
      *(byte *)(extraout_A1 + (short)uVar11) = bVar2;
      uVar11 = uVar11 + 1 & 0x7ff;
      if ((uVar7 & 1) == 0) {
        VDP_DATA = (word)unaff_D7;
      }
      else {
        unaff_D7 = (uint)(ushort)((word)unaff_D7 << 8);
      }
      sVar9 = sVar9 + -1;
      uVar8 = uVar7 - 1;
    } while (sVar9 != -1);
    uVar8 = uVar7 - 1;
  } while (uVar8 != 0xffff);
LAB_00001604:
  return CONCAT24(uVar1 + 1,in_D1);
}


