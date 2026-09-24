// Ghidra headless export
// Entry: 0x00001184
// Name: FUN_00001184
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C FUN_00000e9c


undefined8 FUN_00001184(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  ushort unaff_D2w;
  short unaff_D3w;
  ushort *extraout_A0;
  ushort *extraout_A1;
  ushort *puVar1;
  
  if ((short)in_D1 != 0) {
    VDP_CTRL = 0x8f80;
  }
  FUN_00000e9c();
  puVar1 = extraout_A1;
  do {
    *extraout_A0 = unaff_D2w | *puVar1;
    unaff_D3w = unaff_D3w + -1;
    puVar1 = puVar1 + 1;
  } while (unaff_D3w != -1);
  VDP_CTRL = 0x8f02;
  return CONCAT44(in_D0,in_D1);
}


