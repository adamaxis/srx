// Ghidra headless export
// Entry: 0x00014914
// Name: FUN_00014914
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014902 compute_obstruction_grid_index
//   0x0001494C is_obstruction_path_blocked


void FUN_00014914(void)

{
  undefined1 uVar1;
  
  compute_obstruction_grid_index();
  uVar1 = 0;
  compute_obstruction_grid_index();
  is_obstruction_path_blocked();
  if (!(bool)uVar1) {
    is_obstruction_path_blocked();
    if (!(bool)uVar1) {
      return;
    }
  }
  return;
}


