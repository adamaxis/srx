// Ghidra headless export
// Entry: 0x0000ECCC
// Name: convert_graphics_routine_0x0000eccc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001E2E0 begin_world_activity_if_idle
//   0x0001E2EC begin_world_activity


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_graphics_routine_0x0000eccc(void)

{
  byte bVar1;
  int iVar2;
  int in_A0;
  int extraout_A0;
  int in_A1;
  int extraout_A1;
  
  if ((*(byte *)(in_A1 + 0x9e) & 0x10) == 0) {
    if ((*(byte *)(in_A0 + 0x9e) & 0x10) == 0) goto LAB_0000ed3c;
    bVar1 = *(byte *)(in_A1 + 0x10);
    iVar2 = in_A0;
  }
  else {
    bVar1 = *(byte *)(in_A0 + 0x10);
    iVar2 = in_A1;
  }
  if (((bVar1 & 2) != 0) && (_DAT_00ffe128 = iVar2, DAT_00ffe176 == '\0')) {
    _DAT_00ffdd2c = &UNK_00021174;
    begin_world_activity_if_idle();
    in_A0 = extraout_A0;
    in_A1 = extraout_A1;
    if ((short)(DAT_00ffddc4 + -10) == 0) {
      DAT_00fff0ae = 4;
      if (DAT_00fffbf9 == '\x06') {
        DAT_00fff0ae = 2;
      }
    }
    else {
      DAT_00fff0ae = (&DAT_0000eefe)[(short)(DAT_00ffddc4 + -10)];
    }
  }
LAB_0000ed3c:
  if ((*(byte *)(in_A1 + 0x9e) & 8) == 0) {
    if ((*(byte *)(in_A0 + 0x9e) & 8) == 0) {
      return;
    }
    if ((*(byte *)(in_A0 + 0xb0) & 0x80) != 0) {
      return;
    }
    if ((*(byte *)(in_A1 + 0x10) & 2) == 0) {
      return;
    }
    *(byte *)(in_A0 + 0xb0) = *(byte *)(in_A0 + 0xb0) | 0x80;
  }
  else {
    if (((*(byte *)(in_A1 + 0xb0) & 0x80) != 0) || ((*(byte *)(in_A0 + 0x10) & 2) == 0)) {
      return;
    }
    *(byte *)(in_A1 + 0xb0) = *(byte *)(in_A1 + 0xb0) | 0x80;
  }
  DAT_00fffc3c = DAT_00ffe110;
  _DAT_00ffdd2c = &UNK_0001f146;
  if (DAT_00ffe180 == '\0') {
    if (DAT_00ffe182 != '\0') {
      DAT_00ffe182 = '\0';
      _DAT_00ffdd2c = &UNK_00054402;
    }
  }
  else {
    DAT_00ffe180 = '\0';
    _DAT_00ffdd2c = &UNK_00053c90;
  }
  begin_world_activity();
  return;
}


