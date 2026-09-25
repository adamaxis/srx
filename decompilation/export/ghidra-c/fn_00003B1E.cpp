// Ghidra headless export
// Entry: 0x00003B1E
// Name: convert_field_routine_0x00003b1e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void convert_field_routine_0x00003b1e(void)

{
  ushort uVar1;
  ushort unaff_D4w;
  ushort unaff_D7w;
  
  uVar1 = unaff_D7w - unaff_D4w;
  if (unaff_D7w < unaff_D4w) {
    if (0x2ff < (ushort)-uVar1) {
      return;
    }
    if (0x17f < (ushort)-uVar1) {
      if (DAT_00ffe174 == '\x02') {
        return;
      }
      return;
    }
    if (DAT_00ffe174 != '\x01') {
      return;
    }
    return;
  }
  if (0x2ff < uVar1) {
    return;
  }
  if (0x17f < uVar1) {
    if (DAT_00ffe174 == -2) {
      return;
    }
    return;
  }
  if (DAT_00ffe174 != -1) {
    return;
  }
  return;
}


