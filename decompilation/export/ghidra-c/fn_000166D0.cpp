// Ghidra headless export
// Entry: 0x000166D0
// Name: project_entity_y_axis
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


short project_entity_y_axis(void)

{
  ushort uVar1;
  ushort *in_A0;
  
  if ((in_A0[0x17] < 0x4000) || (0xbfff < in_A0[0x17])) {
    uVar1 = (in_A0[2] >> 4) - 0x10;
  }
  else {
    uVar1 = (in_A0[2] >> 4) + 0x10;
  }
  return ((uVar1 >> 5) - 1) * DAT_00ffddea + (*in_A0 >> 9) * 2;
}


