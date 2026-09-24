// Ghidra headless export
// Entry: 0x0001962A
// Name: FUN_0001962a
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0001962a(void)

{
  ushort uVar1;
  
  uVar1 = (ushort)DAT_00fff0d4;
  if ((&DAT_00fffbd8)[(short)uVar1] == '\x03') {
    DAT_00ff1592 = DAT_00ff1592 & 0xdf;
    DAT_00fff0d9 = 0;
  }
  else if ((&DAT_00fffbd8)[(short)uVar1] == '\x05') {
    DAT_00fff0d2 = 0;
  }
  else if ((&DAT_00fffbd8)[(short)uVar1] == '\x06') {
    DAT_00ff1592 = DAT_00ff1592 & 0xfb;
  }
  else if ((&DAT_00fffbd8)[(short)uVar1] == '\a') {
    DAT_00ff1592 = DAT_00ff1592 & 0xfd;
    DAT_00ffe169 = 0;
  }
  (&DAT_00fffbd8)[(short)uVar1] = 0xff;
  (&DAT_00fff0ed)[(short)uVar1] = 6;
  write_repeated_vdp_grid();
  write_vdp_grid();
  FUN_0001792e();
  return;
}


