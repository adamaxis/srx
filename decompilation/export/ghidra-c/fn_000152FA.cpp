// Ghidra headless export
// Entry: 0x000152FA
// Name: prepare_special_spawn_record
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void prepare_special_spawn_record(void)

{
  undefined2 *in_A1;
  
  *(undefined1 *)((int)in_A1 + 0x41) = 2;
  in_A1[0x2c] = *in_A1;
  in_A1[0x2d] = in_A1[2];
  *(undefined1 *)((int)in_A1 + 0x8b) = 1;
  *(undefined1 *)((int)in_A1 + 0x89) = 0x20;
  return;
}


