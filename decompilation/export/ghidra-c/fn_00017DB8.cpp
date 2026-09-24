// Ghidra headless export
// Entry: 0x00017DB8
// Name: FUN_00017db8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0001792E FUN_0001792e


void FUN_00017db8(void)

{
  char in_D1b;
  bool bVar1;
  
  bVar1 = SCARRY1(in_D1b,DAT_00fffbca);
  DAT_00fffbca = in_D1b + DAT_00fffbca;
  if (DAT_00fffbca == '\0' || bVar1 != DAT_00fffbca < '\0') {
    DAT_00fffbca = '\0';
  }
  else if ('\n' < DAT_00fffbca) {
    DAT_00fffbca = '\n';
  }
  write_vdp_grid();
  FUN_0001792e();
  return;
}


