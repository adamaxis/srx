// Ghidra headless export
// Entry: 0x00000E34
// Name: reset_vdp_upload_state
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 reset_vdp_upload_state(void)

{
  undefined2 in_D0w;
  char in_XF;
  char in_NF;
  char in_ZF;
  char in_VF;
  byte in_CF;
  
  make_vdp_transfer_command(in_XF << 4 | in_NF << 3 | in_ZF << 2 | in_VF << 1 | in_CF);
  Ram00c00000 = 0;
  Ram00c00000 = 0;
  _DAT_00ffdd18 = &DAT_00ffd986;
  _DAT_00ffdd1c = 1;
  _DAT_00ffd97e = 0;
  return in_D0w;
}


