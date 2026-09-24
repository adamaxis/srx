// Ghidra headless export
// Entry: 0x00011ABE
// Name: FUN_00011abe
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00011AE4 FUN_00011ae4
//   0x00012CEE FUN_00012cee


undefined8 FUN_00011abe(void)

{
  undefined8 uVar1;
  
  uVar1 = FUN_00011ae4();
  FUN_00012cee();
  FUN_00012cee();
  return uVar1;
}


