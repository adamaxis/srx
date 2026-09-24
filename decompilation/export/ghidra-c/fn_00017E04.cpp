// Ghidra headless export
// Entry: 0x00017E04
// Name: FUN_00017e04
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 FUN_00001232
//   0x00017C3C FUN_00017c3c


void FUN_00017e04(void)

{
  char in_NF;
  undefined1 in_ZF;
  char in_VF;
  
  FUN_00017c3c();
  if (!(bool)in_ZF && in_VF == in_NF) {
    FUN_00001232();
    FUN_00001232();
    FUN_00001232();
  }
  return;
}


