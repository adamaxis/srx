// Ghidra headless export
// Entry: 0x00059318
// Name: FUN_00059318
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x000597EE FUN_000597ee


void FUN_00059318(void)

{
  short sVar1;
  undefined1 *puVar2;
  undefined1 *puVar3;
  
  sVar1 = 0x13;
  puVar2 = &DAT_00059972;
  puVar3 = &DAT_00fff1e8;
  do {
    *puVar3 = *puVar2;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  } while (sVar1 != -1);
  FUN_000597ee();
  sVar1 = 0xb;
  do {
    FUN_000597ee();
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  DAT_00fffc0c = 0xff;
  DAT_00fffc0d = 0xff;
  return;
}


