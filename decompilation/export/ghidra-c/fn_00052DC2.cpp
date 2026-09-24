// Ghidra headless export
// Entry: 0x00052DC2
// Name: FUN_00052dc2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00052dc2(void)

{
  byte bVar1;
  
  IO_Z80BUS = 0x100;
  do {
    bVar1 = IO_Z80BUS._0_1_;
  } while ((bVar1 & 1) != 0);
  return;
}


