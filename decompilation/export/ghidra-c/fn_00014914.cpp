// Ghidra headless export
// Entry: 0x00014914
// Name: FUN_00014914
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014902 compute_obstruction_grid_index
//   0x0001494C FUN_0001494c


void FUN_00014914(void)

{
  undefined1 uVar1;
  
  compute_obstruction_grid_index();
  uVar1 = 0;
  compute_obstruction_grid_index();
  FUN_0001494c();
  if (!(bool)uVar1) {
    FUN_0001494c();
    if (!(bool)uVar1) {
      return;
    }
  }
  return;
}


