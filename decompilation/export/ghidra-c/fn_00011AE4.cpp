// Ghidra headless export
// Entry: 0x00011AE4
// Name: FUN_00011ae4
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text


undefined8 FUN_00011ae4(void)

{
  char cVar1;
  int iVar2;
  undefined4 in_D0;
  undefined4 in_D1;
  
  iVar2 = DAT_00fffc04;
  cVar1 = (char)in_D0 + *(char *)(DAT_00fffc04 + 0x4d);
  *(char *)(DAT_00fffc04 + 0x4d) = cVar1;
  if (cVar1 < '\0') {
    *(undefined1 *)(iVar2 + 0x4d) = 0;
  }
  else if ('\x04' < *(char *)(iVar2 + 0x4d)) {
    *(undefined1 *)(iVar2 + 0x4d) = 4;
  }
  write_vdp_grid();
  render_encoded_title_text();
  return CONCAT44(in_D0,in_D1);
}


