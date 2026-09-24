// Ghidra headless export
// Entry: 0x00019856
// Name: refresh_field_hud
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00017DB8 FUN_00017db8


void refresh_field_hud(void)

{
  short sVar1;
  
  FUN_00017db8();
  sVar1 = 4;
  do {
    write_vdp_grid();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


