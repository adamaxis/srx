// Ghidra headless export
// Entry: 0x00059BCC
// Name: FUN_00059bcc
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003674 FUN_00003674
//   0x000037EC FUN_000037ec


void FUN_00059bcc(void)

{
  short sVar1;
  undefined2 *puVar2;
  int extraout_A0;
  
  puVar2 = &DAT_00ff0100;
  sVar1 = 2;
  do {
    *(byte *)(puVar2 + 8) = *(byte *)(puVar2 + 8) & 0xbf;
    *(byte *)(puVar2 + 8) = *(byte *)(puVar2 + 8) & 0xef;
    FUN_00003674();
    FUN_000037ec();
    puVar2 = (undefined2 *)(extraout_A0 + 0x100);
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  DAT_00ff014c = 10;
  DAT_00ff019c = 10;
  DAT_00ff024c = 10;
  DAT_00ff029c = 10;
  DAT_00ff034c = 10;
  DAT_00ff039c = 10;
  return;
}


