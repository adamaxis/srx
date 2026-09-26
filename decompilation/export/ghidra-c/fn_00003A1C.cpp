// Ghidra headless export
// Entry: 0x00003A1C
// Name: update_field_camera
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003A90 accept_camera_delta
//   0x00003B1E convert_field_routine_0x00003b1e
//   0x00003BAC convert_field_routine_0x00003bac
//   0x00015DCC transform_entity_heading_vector


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void update_field_camera(void)

{
  ushort uVar1;
  undefined1 extraout_D1b;
  short extraout_D1w;
  short sVar2;
  ushort uVar3;
  int extraout_A0;
  undefined1 auStack_6 [2];
  undefined4 uStack_4;
  
  if (DAT_00ffe186 == '\x03') {
    DAT_00ffdf5e = 0;
    DAT_00ffdf5c = 0x600;
    convert_field_routine_0x00003bac();
    return;
  }
  sVar2 = *(short *)(DAT_00fffc04 + 0x2e);
  uStack_4 = 0x3a50;
  transform_entity_heading_vector();
  uVar3 = *(short *)(extraout_A0 + 4) + sVar2;
  if ((short)uVar3 < 0) {
    uVar3 = 0;
  }
  uStack_4 = 0x3a7c;
  accept_camera_delta();
  uStack_4 = 0x3a86;
  DAT_00ffe173 = extraout_D1b;
  convert_field_routine_0x00003b1e();
  DAT_00ffe174 = (undefined1)uVar3;
  if (_DAT_00ffe100 != 0) {
    return;
  }
  if (_DAT_00ffe102 != 0) {
    return;
  }
  if (_DAT_00ffdfe8 != 0) {
    return;
  }
  sVar2 = extraout_D1w * 0x10;
  if (sVar2 != 0) {
    if (sVar2 < 0) {
      if ((short)(DAT_00ffddc6 + sVar2) < DAT_00ffddca) goto LAB_00003cba;
    }
    else if (DAT_00ffddce < (short)(DAT_00ffddc6 + sVar2)) goto LAB_00003cba;
    DAT_00ffddc6 = DAT_00ffddc6 + sVar2;
    DAT_00ffdd08 = -extraout_D1w + DAT_00ffdd08;
    DAT_00ffdd0c = -extraout_D1w + DAT_00ffdd0c;
    uVar1 = DAT_00ffddc6 & 0xff80;
    if (uVar1 != DAT_00ffdf38) {
      uStack_4 = CONCAT22(uVar3,uStack_4._2_2_);
      DAT_00ffdf38 = uVar1;
      render_field_tile_row_edge();
      register0x0000003c = (BADSPACEBASE *)auStack_6;
      uVar3 = uVar1;
    }
  }
LAB_00003cba:
  sVar2 = uVar3 * 0x10;
  if (sVar2 != 0) {
    if (sVar2 < 0) {
      if ((short)(DAT_00ffddc8 + sVar2) < DAT_00ffddcc) {
        return;
      }
    }
    else if (DAT_00ffddd0 < (short)(DAT_00ffddc8 + sVar2)) {
      return;
    }
    DAT_00ffddc8 = DAT_00ffddc8 + sVar2;
    DAT_00ffdd0a = uVar3 + DAT_00ffdd0a;
    DAT_00ffdd0e = uVar3 + DAT_00ffdd0e;
    if ((DAT_00ffddc8 & 0xff80) != DAT_00ffdf36) {
      *(undefined4 *)((int)register0x0000003c + -4) = 0x3d16;
      DAT_00ffdf36 = DAT_00ffddc8 & 0xff80;
      render_field_tile_edge();
    }
  }
  return;
}

