// Ghidra headless export
// Entry: 0x00005A62
// Name: project_entity_screen_position_standard
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


ushort project_entity_screen_position_standard(void)

{
  ushort uVar1;
  ushort uVar2;
  short sVar3;
  short *in_A0;
  
  sVar3 = (short)(*in_A0 - DAT_00ffddc6) >> 4;
  if (((ushort)(*in_A0 - DAT_00ffddc6) >> 3 & 1) != 0) {
    sVar3 = sVar3 + 1;
  }
  uVar1 = sVar3 + 0x80;
  in_A0[5] = uVar1;
  sVar3 = (short)(in_A0[2] - DAT_00ffddc8) >> 4;
  if (((ushort)(in_A0[2] - DAT_00ffddc8) >> 3 & 1) != 0) {
    sVar3 = sVar3 + 1;
  }
  uVar2 = in_A0[4] + sVar3 + 0x80;
  in_A0[6] = uVar2;
  if ((((0x6f < (short)uVar1) && (uVar1 < 0x1ae)) && (0x62 < (short)uVar2)) && (uVar2 < 0x17a)) {
    return 1;
  }
  in_A0[6] = 0;
  return uVar1;
}


