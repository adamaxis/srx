// Ghidra headless export
// Entry: 0x00013AF2
// Name: find_first_entity_action_below_threshold
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


short find_first_entity_action_below_threshold(void)

{
  short in_D0w;
  short sVar1;
  int in_A0;
  undefined1 *puVar2;
  
  sVar1 = 7;
  puVar2 = (undefined1 *)(in_A0 + 0x66);
  do {
    in_D0w = CONCAT11((char)((ushort)in_D0w >> 8),*puVar2);
    if (in_D0w < 0xd) {
      return in_D0w;
    }
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  return 0;
}


