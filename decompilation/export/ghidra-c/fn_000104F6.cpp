// Ghidra headless export
// Entry: 0x000104F6
// Name: apply_action_item_phase_modifiers
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void apply_action_item_phase_modifiers(void)

{
  short sVar1;
  char unaff_D5b;
  int in_A0;
  
  sVar1 = 6;
  do {
    if (unaff_D5b == *(char *)(in_A0 + 0x66 + (int)sVar1)) {
      return;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


