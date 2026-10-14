// Ghidra headless export
// Entry: 0x00059786
// Name: sum_game_save_payload
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined1 sum_game_save_payload(void)

{
  undefined1 uVar1;
  short sVar2;
  undefined1 *in_A1;
  
  sVar2 = 0x61b;
  do {
    uVar1 = *in_A1;
    sVar2 = sVar2 + -1;
    in_A1 = in_A1 + 1;
  } while (sVar2 != -1);
  return uVar1;
}


