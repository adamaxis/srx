// Ghidra headless export
// Entry: 0x00055ED6
// Name: calculate_equipment_command_average
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void calculate_equipment_command_average(void)

{
  short sVar1;
  int in_A0;
  
  sVar1 = 7;
  do {
    if (*(char *)(in_A0 + 0x66) == '(') {
      return;
    }
    in_A0 = in_A0 + 1;
    sVar1 = sVar1 + -1;
  } while (sVar1 != -1);
  return;
}


