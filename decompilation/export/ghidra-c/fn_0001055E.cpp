// Ghidra headless export
// Entry: 0x0001055E
// Name: FUN_0001055e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0001047E refresh_ready_entity_animations
//   0x000104C8 find_matching_menu_metric
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
    find_matching_menu_metric();
    cVar1 = '\0';
  }
  else {
    refresh_ready_entity_animations();
    cVar1 = *(char *)(extraout_A0 + 0x7c);
  }
  while (write_vdp_grid(), cVar1 != '\0') {
    cVar1 = '\0';
  }
  return;
}


