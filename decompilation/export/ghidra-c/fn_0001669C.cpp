// Ghidra headless export
// Entry: 0x0001669C
// Name: project_entity_x_axis
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


short project_entity_x_axis(void)

{
  ushort uVar1;
  ushort *in_A0;
  
  if (in_A0[0x17] < 0x8000) {
    uVar1 = (*in_A0 >> 4) + 0x10;
  }
  else {
    uVar1 = (*in_A0 >> 4) - 0x10;
  }
  return ((in_A0[2] >> 9) - 1) * DAT_00ffddea + (uVar1 >> 5) * 2;
}


