// Ghidra headless export
// Entry: 0x00055B16
// Name: FUN_00055b16
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D5A FUN_00000d5a
//   0x00000DBA count_thresholded_random_rolls
//   0x0001047E refresh_ready_entity_animations
//   0x00055740 FUN_00055740
//   0x00055786 FUN_00055786
//   0x000559BC FUN_000559bc


/* WARNING: Removing unreachable block (ram,0x00055b36) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00055b16(void)

{
  int extraout_A0;
  int extraout_A1;
  
  FUN_00055740();
  FUN_00055786();
  FUN_00000d5a();
  count_thresholded_random_rolls();
  *(undefined1 *)(extraout_A0 + 0x9f) = DAT_00fff0dd;
  *(undefined1 *)(extraout_A0 + 0x9d) = 6;
  if ((short)(*(short *)(extraout_A1 + 0x3e) + -3) < 0) {
    refresh_ready_entity_animations();
  }
  _DAT_00fff112 = 1;
  FUN_000559bc();
  return;
}


