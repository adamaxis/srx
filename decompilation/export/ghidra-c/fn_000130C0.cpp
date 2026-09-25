// Ghidra headless export
// Entry: 0x000130C0
// Name: convert_menu_routine_0x000130c0
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x00012A54 update_resource_variant


undefined8 convert_menu_routine_0x000130c0(void)

{
  undefined4 in_D0;
  char cVar1;
  undefined4 in_D1;
  char *in_A0;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  int extraout_A0_04;
  int iVar2;
  
  cVar1 = *in_A0;
  update_resource_variant();
  write_vdp_grid();
  if ((char)(cVar1 + -0xc) < '\0') {
    iVar2 = extraout_A0;
    if ((*(byte *)(extraout_A0 + 1) & 2) != 0) {
      cVar1 = write_vdp_grid();
      iVar2 = extraout_A0_00;
    }
    if ((*(byte *)(iVar2 + 1) & 4) != 0) {
      cVar1 = write_vdp_grid();
      iVar2 = extraout_A0_01;
    }
    if ((*(byte *)(iVar2 + 1) & 1) != 0) {
      cVar1 = write_vdp_grid();
      iVar2 = extraout_A0_02;
    }
    if ((*(byte *)(iVar2 + 1) & 8) != 0) {
      cVar1 = write_vdp_grid();
      iVar2 = extraout_A0_03;
    }
    if ((*(byte *)(iVar2 + 1) & 0x10) != 0) {
      cVar1 = write_vdp_grid();
      iVar2 = extraout_A0_04;
    }
    if ((*(byte *)(iVar2 + 1) & 0x20) != 0) {
      cVar1 = write_vdp_grid();
    }
  }
  if (((((char)(cVar1 + -0xc) < '\0') || ((char)(cVar1 + -0x13) < '\0')) || (cVar1 == '\x13')) ||
     ((-1 < (char)(cVar1 + -0x17) && ((char)(cVar1 + -0x28) < '\0')))) {
    render_encoded_title_text();
    write_vdp_grid();
  }
  return CONCAT44(in_D0,in_D1);
}


