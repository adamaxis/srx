// Ghidra headless export
// Entry: 0x0001253E
// Name: FUN_0001253e
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA FUN_00000dba
//   0x00003674 FUN_00003674
//   0x000037EC FUN_000037ec


void FUN_0001253e(void)

{
  int extraout_A0;
  int in_A1;
  
  if ((*(char *)(in_A1 + 0x4c) == '\n') && (*(char *)(in_A1 + 0x9c) == '\n')) {
    return;
  }
  FUN_00000dba();
  FUN_00003674();
  FUN_000037ec();
  *(byte *)(extraout_A0 + 0x10) = *(byte *)(extraout_A0 + 0x10) & 0xbf;
  *(byte *)(extraout_A0 + 0x10) = *(byte *)(extraout_A0 + 0x10) & 0xef;
  *(byte *)(extraout_A0 + 0x10) = *(byte *)(extraout_A0 + 0x10) | 1;
  *(undefined4 *)(extraout_A0 + 0x3a) = 0xb45c;
  return;
}


