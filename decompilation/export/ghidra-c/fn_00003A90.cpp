// Ghidra headless export
// Entry: 0x00003A90
// Name: FUN_00003a90
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00003a90(void)

{
  ushort uVar1;
  ushort unaff_D3w;
  ushort unaff_D6w;
  
  uVar1 = unaff_D6w - unaff_D3w;
  if (unaff_D6w < unaff_D3w) {
    if (0x2ff < (ushort)-uVar1) {
      return;
    }
    if (0x17f < (ushort)-uVar1) {
      if (DAT_00ffe173 == '\x02') {
        return;
      }
      return;
    }
    if (DAT_00ffe173 != '\x01') {
      return;
    }
    return;
  }
  if (0x2ff < uVar1) {
    return;
  }
  if (0x17f < uVar1) {
    if (DAT_00ffe173 == -2) {
      return;
    }
    return;
  }
  if (DAT_00ffe173 != -1) {
    return;
  }
  return;
}


