// Ghidra headless export
// Entry: 0x000033BE
// Name: FUN_000033be
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000F5C FUN_00000f5c


undefined8 FUN_000033be(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short sVar1;
  short sVar2;
  
  sVar2 = 3;
  do {
    sVar1 = 3;
    do {
      FUN_00000f5c();
      sVar1 = sVar1 + -1;
    } while (sVar1 != -1);
    sVar2 = sVar2 + -1;
  } while (sVar2 != -1);
  return CONCAT44(in_D0,in_D1);
}


