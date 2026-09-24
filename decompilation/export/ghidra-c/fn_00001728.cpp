// Ghidra headless export
// Entry: 0x00001728
// Name: render_hud_sprite_stream
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C FUN_00000e9c


void render_hud_sprite_stream(void)

{
  short sVar1;
  short sVar2;
  short sVar3;
  short extraout_D1w;
  short extraout_D1w_00;
  short extraout_D1w_01;
  short extraout_D1w_02;
  short sVar4;
  ushort unaff_D2w;
  short unaff_D3w;
  short sVar5;
  byte bVar6;
  byte bVar7;
  short unaff_D5w;
  ushort uVar8;
  short unaff_D7w;
  short *in_A1;
  byte *extraout_A1;
  byte *pbVar9;
  byte *extraout_A1_00;
  byte *extraout_A1_01;
  byte *pbVar10;
  byte *pbVar11;
  byte *extraout_A1_02;
  int iVar12;
  short sVar13;
  bool bVar14;
  
  sVar1 = *in_A1;
  FUN_00000e9c();
  sVar2 = unaff_D7w - unaff_D5w;
  sVar13 = 0;
  iVar12 = (int)unaff_D3w;
  sVar4 = extraout_D1w;
  pbVar11 = extraout_A1;
  sVar3 = unaff_D7w;
  do {
    while( true ) {
      pbVar9 = pbVar11 + 1;
      bVar6 = *pbVar11;
      if (bVar6 == 0) {
        return;
      }
      if (bVar6 < 0xc1) break;
      if ((bVar6 & 0x20) == 0) {
        bVar6 = bVar6 & 0x1f;
        do {
          pbVar11 = pbVar9 + 1;
          sVar13 = sVar13 + 1;
          if (iVar12 - sVar13 < 0) {
            if (sVar2 < sVar3) {
              VDP_DATA = unaff_D2w | sVar1 + (ushort)*pbVar9;
            }
            sVar3 = sVar3 + -1;
            if (sVar3 == -1) {
              if ((short)(sVar4 + -1) < 0) {
                return;
              }
              FUN_00000e9c();
              sVar4 = extraout_D1w_00;
              pbVar11 = extraout_A1_00;
              sVar3 = unaff_D7w;
            }
          }
          bVar14 = SBORROW1(bVar6,'\x01');
          bVar6 = bVar6 - 1;
          pbVar9 = pbVar11;
        } while (bVar6 != 0 && bVar14 == (short)((ushort)bVar6 << 8) < 0);
      }
      else {
        bVar6 = bVar6 & 0x1f;
        do {
          pbVar11 = pbVar9 + 2;
          sVar13 = sVar13 + 1;
          if (iVar12 - sVar13 < 0) {
            if (sVar2 < sVar3) {
              VDP_DATA = unaff_D2w | sVar1 + *(short *)pbVar9;
            }
            sVar3 = sVar3 + -1;
            if (sVar3 == -1) {
              if ((short)(sVar4 + -1) < 0) {
                return;
              }
              FUN_00000e9c();
              sVar4 = extraout_D1w_01;
              pbVar11 = extraout_A1_01;
              sVar3 = unaff_D7w;
            }
          }
          bVar14 = SBORROW1(bVar6,'\x01');
          bVar6 = bVar6 - 1;
          pbVar9 = pbVar11;
        } while (bVar6 != 0 && bVar14 == (short)((ushort)bVar6 << 8) < 0);
      }
    }
    sVar5 = 1;
    if ((bVar6 < 0x81) && (sVar5 = 0, bVar6 < 0x41)) {
      sVar5 = -1;
    }
    bVar7 = 0;
    pbVar10 = pbVar9;
    if ((bVar6 & 0x20) != 0) {
      pbVar10 = pbVar11 + 2;
      bVar7 = *pbVar9;
    }
    bVar6 = bVar6 & 0x1f;
    pbVar11 = pbVar10 + 1;
    uVar8 = sVar1 + CONCAT11(bVar7,*pbVar10);
    do {
      sVar13 = sVar13 + 1;
      if (iVar12 - sVar13 < 0) {
        if (sVar2 < sVar3) {
          uVar8 = unaff_D2w | uVar8;
          VDP_DATA = uVar8;
        }
        sVar3 = sVar3 + -1;
        if (sVar3 == -1) {
          if ((short)(sVar4 + -1) < 0) {
            return;
          }
          FUN_00000e9c();
          sVar4 = extraout_D1w_02;
          pbVar11 = extraout_A1_02;
          sVar3 = unaff_D7w;
        }
      }
      uVar8 = sVar5 + uVar8;
      bVar14 = SBORROW1(bVar6,'\x01');
      bVar6 = bVar6 - 1;
    } while (bVar14 == (short)((ushort)bVar6 << 8) < 0);
  } while( true );
}


