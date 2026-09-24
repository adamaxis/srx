// Ghidra headless export
// Entry: 0x0001055E
// Name: FUN_0001055e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0001047E FUN_0001047e
//   0x000104C8 FUN_000104c8
//   0x00055F02 FUN_00055f02
//   0x00055F20 FUN_00055f20


void FUN_0001055e(void)

{
  short in_D0w;
  char cVar1;
  int extraout_A0;
  
  if (in_D0w == 0) {
    FUN_00055f02();
    FUN_00055f20();
    FUN_000104c8();
    cVar1 = '\0';
  }
  else {
    FUN_0001047e();
    cVar1 = *(char *)(extraout_A0 + 0x7c);
  }
  while (write_vdp_grid(), cVar1 != '\0') {
    cVar1 = '\0';
  }
  return;
}


