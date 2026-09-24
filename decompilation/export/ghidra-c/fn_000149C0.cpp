// Ghidra headless export
// Entry: 0x000149C0
// Name: FUN_000149c0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014756 clear_entity_route_flags
//   0x00014764 FUN_00014764
//   0x000148AA FUN_000148aa
//   0x00014902 compute_obstruction_grid_index
//   0x00014914 FUN_00014914
//   0x00014B4E decode_obstruction_tile_projection
//   0x00014C2E derive_entity_axis_relation_flags
//   0x00015CA4 compute_diagonal_step_cost


void FUN_000149c0(void)

{
  short sVar1;
  undefined2 uVar2;
  short sVar3;
  short extraout_D1w;
  short extraout_D1w_00;
  short sVar4;
  ushort unaff_D2w;
  ushort uVar5;
  int extraout_A0;
  ushort *in_A0;
  ushort *extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  int extraout_A0_04;
  int extraout_A0_05;
  int iVar6;
  short *psVar7;
  undefined1 in_CF;
  
  if (((in_A0[0x58] & 0x800) != 0) && (FUN_00014914(), in_A0 = extraout_A0_00, (bool)in_CF)) {
    *(byte *)(extraout_A0_00 + 0x58) = *(byte *)(extraout_A0_00 + 0x58) & 0xfd;
    *(byte *)(extraout_A0_00 + 0x58) = *(byte *)(extraout_A0_00 + 0x58) & 0xf7;
    return;
  }
  sVar1 = (in_A0[2] >> 4 & 0x7fe0) * 2 + (*in_A0 >> 9);
  psVar7 = (short *)((int)in_A0 + (short)in_A0[0x5d] + 0xbc);
  if (sVar1 == *psVar7) {
    if (psVar7[1] == -1) {
      if ((in_A0[0x58] & 0x800) != 0) {
        compute_obstruction_grid_index();
        uVar5 = *(ushort *)(extraout_A0_03 + 0x34) >> 0xd & 6;
        if (((&DAT_00ff1700)[(short)(*(short *)(&DAT_000148ca + (short)uVar5) + sVar1)] == '\0') &&
           ((&DAT_00ff1700)
            [(short)(*(short *)(&DAT_000148ca + (short)(uVar5 + 2 & 6)) +
                    *(short *)(&DAT_000148ca + (short)(uVar5 + 4 & 6)) + sVar1)] == '\0')) {
          FUN_000148aa();
          sVar4 = extraout_D1w;
          iVar6 = extraout_A0_04;
        }
        else {
          sVar3 = 4;
          while (FUN_00014764(), sVar4 = extraout_D1w_00, iVar6 = extraout_A0_05,
                extraout_D1w_00 == sVar1) {
            sVar3 = sVar3 + -1;
            if (sVar3 == -1) {
              clear_entity_route_flags();
              return;
            }
          }
        }
        *(undefined2 *)(iVar6 + 0xba) = 0;
        *(short *)(iVar6 + 0xbc) = sVar4;
        *(undefined2 *)(iVar6 + 0xbe) = 0xffff;
        return;
      }
      *(byte *)(in_A0 + 0x58) = *(byte *)(in_A0 + 0x58) & 0xfd;
      *(byte *)(in_A0 + 0x58) = *(byte *)(in_A0 + 0x58) & 0xf7;
      derive_entity_axis_relation_flags();
      uVar2 = *(undefined2 *)(extraout_A0_02 + 0x96);
      uVar5 = *(ushort *)(extraout_A0_02 + 0x98);
      FUN_00015d3c();
      iVar6 = extraout_A0;
      goto LAB_000145fc;
    }
    in_A0[0x5d] = in_A0[0x5d] + 2;
  }
  decode_obstruction_tile_projection();
  uVar5 = unaff_D2w << 0xd | unaff_D2w >> 3;
  uVar2 = compute_diagonal_step_cost();
  iVar6 = extraout_A0_01;
LAB_000145fc:
  *(ushort *)(iVar6 + 0x34) = uVar5;
  *(undefined2 *)(iVar6 + 200) = uVar2;
  *(undefined1 **)(iVar6 + 0x3a) = &LAB_0000b574;
  return;
}


