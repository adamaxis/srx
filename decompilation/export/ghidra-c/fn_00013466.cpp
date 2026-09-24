// Ghidra headless export
// Entry: 0x00013466
// Name: FUN_00013466
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003404 FUN_00003404
//   0x000036FC FUN_000036fc


void FUN_00013466(void)

{
  char in_D0b;
  undefined1 unaff_D7b;
  int in_A0;
  
  *(char *)(in_A0 + 0x57) = in_D0b;
  if (in_D0b == '\0') {
    *(undefined1 *)(in_A0 + 0x57) = 0;
    *(undefined1 *)(in_A0 + 0x9d) = 0;
    *(undefined1 *)(in_A0 + 0x56) = VectorsTable_00000000.IRQ7._3_1_;
  }
  else {
    *(undefined1 *)(in_A0 + 0x56) = unaff_D7b;
    if (*(char *)(in_A0 + 0x4e) != '\0') {
      *(undefined1 *)(in_A0 + 0x4e) = 0;
      *(undefined1 *)(in_A0 + 0x9d) = 0;
    }
  }
  FUN_000036fc();
  FUN_00003404();
  return;
}


