// Ghidra headless export
// Entry: 0x00014226
// Name: FUN_00014226
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000036FC FUN_000036fc
//   0x0000ABFC FUN_0000abfc
//   0x000530DE FUN_000530de


void FUN_00014226(void)

{
  int extraout_A0;
  
  FUN_000530de();
  FUN_0000abfc();
  *(char *)(extraout_A0 + 0x4f) = *(char *)(extraout_A0 + 0x4f) + -1;
  FUN_000036fc();
  return;
}


