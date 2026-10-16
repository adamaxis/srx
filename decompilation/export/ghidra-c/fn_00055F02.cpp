// Ghidra headless export
// Entry: 0x00055F02
// Name: calculate_item18_skill_value
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void calculate_item18_skill_value(void)

{
  short sVar1;
  int in_A0;
  
  sVar1 = 6;
  do {
    if (*(char *)(in_A0 + 0x66 + (int)sVar1) == '\x18') {
      return;
    }
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


