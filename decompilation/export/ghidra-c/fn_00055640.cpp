// Ghidra headless export
// Entry: 0x00055640
// Name: FUN_00055640
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D5A FUN_00000d5a
//   0x00000DBA FUN_00000dba
//   0x00012C74 FUN_00012c74
//   0x000556C6 FUN_000556c6
//   0x00055740 FUN_00055740
//   0x00055786 FUN_00055786
//   0x00055ED6 FUN_00055ed6
//   0x00055F20 FUN_00055f20


undefined2 FUN_00055640(void)

{
  undefined2 in_D0w;
  int extraout_A0;
  int extraout_A1;
  
  FUN_00055ed6();
  FUN_00055f20();
  FUN_00012c74();
  FUN_00055740();
  FUN_00055786();
  FUN_000556c6();
  FUN_00000d5a();
  FUN_00000dba();
  *(undefined1 *)(extraout_A0 + 0x9f) = DAT_00fff0dd;
  *(undefined1 *)(extraout_A1 + 0x9f) = DAT_00fff0dd;
  return in_D0w;
}


