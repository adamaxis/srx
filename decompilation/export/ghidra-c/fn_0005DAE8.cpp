// Ghidra headless export
// Entry: 0x0005DAE8
// Name: convert_intro_routine_0x0005dae8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0005DAFC convert_intro_routine_0x0005dafc


void convert_intro_routine_0x0005dae8(void)

{
  short sVar1;
  undefined2 *puVar2;
  
  sVar1 = 0x1bf;
  puVar2 = (undefined2 *)&DAT_00ffb740;
  do {
    *puVar2 = 0;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  convert_intro_routine_0x0005dafc();
  return;
}


