// Ghidra headless export
// Entry: 0x0001A322
// Name: FUN_0001a322
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0001a322(void)

{
  short sVar1;
  undefined *puVar2;
  
  DAT_00ffe177 = 0;
  puVar2 = &DAT_00ff0400;
  sVar1 = 0xc;
  do {
    puVar2[0x10] = puVar2[0x10] & 0xfe;
    puVar2 = puVar2 + 0x100;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


