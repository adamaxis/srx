// Ghidra headless export
// Entry: 0x00052E4E
// Name: FUN_00052e4e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00052e4e(void)

{
  byte bVar1;
  
  IO_Z80BUS = 0x100;
  do {
    bVar1 = IO_Z80BUS._0_1_;
  } while ((bVar1 & 1) != 0);
  return;
}


