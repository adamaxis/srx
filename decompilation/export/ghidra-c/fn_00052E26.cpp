// Ghidra headless export
// Entry: 0x00052E26
// Name: FUN_00052e26
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_00052e26(void)

{
  int iVar1;
  
  IO_Z80RES = 0;
  iVar1 = 0xf;
  do {
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  IO_Z80BUS = 0;
  IO_Z80RES = 0x100;
  return;
}


