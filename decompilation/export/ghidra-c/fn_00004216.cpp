// Ghidra headless export
// Entry: 0x00004216
// Name: offset_field_coordinate
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


short offset_field_coordinate(void)

{
  short sVar1;
  
  sVar1 = (DAT_00ffddc6 >> 4) - 8;
  if ((sVar1 < 0) || (sVar1 = (DAT_00ffddc6 >> 4) - 0x10, sVar1 < 0)) {
    sVar1 = sVar1 + 8;
  }
  return sVar1;
}


