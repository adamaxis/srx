// Ghidra headless export
// Entry: 0x0000B7B8
// Name: FUN_0000b7b8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000423C quantize_field_delta
//   0x0000BC36 build_collision_probe_points
//   0x0001651E FUN_0001651e
//   0x00016760 mark_projected_terrain_cell
//   0x0001E2E0 begin_world_activity_if_idle
//   0x0001E2EC begin_world_activity
//   0x00053126 prepare_mode_entry_service


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0000b7b8(void)

{
  ushort uVar1;
  ushort unaff_D4w;
  int in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  int extraout_A0_04;
  int extraout_A0_05;
  int extraout_A0_06;
  int extraout_A0_07;
  int extraout_A0_08;
  int extraout_A0_09;
  int extraout_A0_10;
  int extraout_A0_11;
  int extraout_A0_12;
  byte bVar2;
  bool bVar3;
  
  if ((_DAT_00ffdfe8 != 0) ||
     (bVar2 = *(byte *)(in_A0 + 0xb0), *(byte *)(in_A0 + 0xb0) = bVar2 | 0x20, (bVar2 & 0x20) != 0))
  {
    return;
  }
  if (((*(byte *)(in_A0 + 0x10) & 2) != 0) &&
     (((DAT_00ffdf40 == 2 || (DAT_00ffddc4 == 0x14)) || (_DAT_00ffdf84 == 0)))) {
    bVar2 = DAT_00ffef10 < '\0';
    if (DAT_00ffef11 < '\0') {
      bVar2 = bVar2 + 1;
    }
    if (DAT_00ffef12 < '\0') {
      bVar2 = bVar2 + 1;
    }
    if (DAT_00ffef13 < '\0') {
      bVar2 = bVar2 + 1;
    }
    if (1 < bVar2) {
      build_collision_probe_points();
      *(ushort *)(extraout_A0 + 0xcc) = unaff_D4w << 0xe | unaff_D4w >> 2;
      quantize_field_delta();
      uVar1 = DAT_00ffdf64;
      *(ushort *)(extraout_A0_00 + 0xc6) = DAT_00ffdf64;
      uVar1 = uVar1 & 0x1f;
      if (uVar1 == 0x1f) {
        _DAT_00ffdd2c = (code *)&UNK_0000db26;
        begin_world_activity_if_idle();
        in_A0 = extraout_A0_01;
      }
      else if (uVar1 == 0x1e) {
        _DAT_00ffdd2c = (code *)&UNK_00020990;
        begin_world_activity_if_idle();
        in_A0 = extraout_A0_02;
      }
      else {
        in_A0 = extraout_A0_00;
        if (uVar1 == 0x1d) {
          if (DAT_00ffe16f == '\0') {
            _DAT_00ffdd2c = (code *)&UNK_00058d94;
            begin_world_activity_if_idle();
            in_A0 = extraout_A0_03;
          }
        }
        else if (uVar1 == 0x1c) {
          if (DAT_00ffe16f == '\0') {
            _DAT_00ffdd2c = (code *)&UNK_00058db2;
            begin_world_activity_if_idle();
            in_A0 = extraout_A0_04;
          }
        }
        else if (uVar1 == 0x1b) {
          _DAT_00ffdd2c = (code *)&UNK_00053d5a;
          begin_world_activity_if_idle();
          in_A0 = extraout_A0_05;
        }
        else if (uVar1 == 0x1a) {
          DAT_00ffe178 = 0;
          bVar2 = (byte)*(undefined2 *)(extraout_A0_00 + 0xc6);
          bVar3 = bVar2 < 0xda;
          if ((bVar2 != 0xda) && ((DAT_00fffc1c & 0x80) == 0)) {
            DAT_00ffe178 = 1;
            bVar3 = false;
          }
          mark_projected_terrain_cell();
          in_A0 = extraout_A0_06;
          if (bVar3) {
            _DAT_00ffdd2c = (code *)&UNK_00020c90;
            begin_world_activity();
            in_A0 = extraout_A0_07;
          }
        }
        else if (((DAT_00ffddc4 == 0x14) || (DAT_00ffdf40 == 2)) &&
                ((1 < uVar1 && ((uVar1 < 6 && (DAT_00ffe17f == '\0')))))) {
          _DAT_00ffdd2c = (code *)&UNK_00020e48;
          DAT_00ffe16e = begin_world_activity_if_idle();
          DAT_00ffe11a = DAT_00ffdfe4;
          in_A0 = extraout_A0_08;
        }
        else if (uVar1 == 0x18) {
          _DAT_00ffdd2c = (code *)&UNK_00020ec2;
          begin_world_activity_if_idle();
          in_A0 = extraout_A0_09;
        }
        else {
          if (uVar1 == 0x19) {
            if (DAT_00fffc10 == 0x13) {
              _DAT_00ffdd2c = (code *)&UNK_00053efa;
              begin_world_activity_if_idle();
              in_A0 = extraout_A0_10;
              goto LAB_0000baae;
            }
            DAT_00ffef10 = -0x58;
            DAT_00ffdf64 = 0xa8;
            *(undefined2 *)(extraout_A0_00 + 0xc6) = 0xa8;
            uVar1 = 8;
          }
          if (((DAT_00ff0110 & 0x40) == 0) && ((DAT_00ff0110 & 0x10) == 0)) {
            DAT_00fffbf9 = (undefined1)uVar1;
            bVar3 = false;
            if (((((DAT_00ffdf40 == 0) || (bVar3 = DAT_00ffddc4 < 0x14, DAT_00ffddc4 == 0x14)) ||
                 (bVar3 = DAT_00ffdf40 < 2, DAT_00ffdf40 != 2)) || (bVar3 = false, uVar1 != 0)) ||
               ((_DAT_00ffdf84 == 0 && (bVar3 = false, DAT_00ffe16f == '\0')))) {
              FUN_0001651e();
              if (bVar3) {
                _DAT_00ffdf3a = 0xffff;
                DAT_00ffdf3c = 0x10;
                _DAT_00ffdfe8 = 0xffff;
                return;
              }
              *(ushort *)(extraout_A0_12 + 0xc6) = DAT_00ffdf64;
              return;
            }
          }
          else {
            prepare_mode_entry_service();
            _DAT_00ffdd2c = FUN_0000c64c;
            begin_world_activity();
            in_A0 = extraout_A0_11;
          }
        }
      }
    }
  }
LAB_0000baae:
  *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) & 0xdf;
  *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) | 1;
  return;
}


