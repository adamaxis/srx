// Ghidra headless export
// Entry: 0x00014914
// Name: FUN_00014914
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014902 FUN_00014902
//   0x0001494C FUN_0001494c


void FUN_00014914(void)

{
  undefined1 uVar1;
  
  FUN_00014902();
  uVar1 = 0;
  FUN_00014902();
  FUN_0001494c();
  if (!(bool)uVar1) {
    FUN_0001494c();
    if (!(bool)uVar1) {
      return;
    }
  }
  return;
}


