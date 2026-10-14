// Ghidra headless export
// Entry: 0x0001B5A6
// Name: reset_word_stream_area
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void reset_word_stream_area(void)

{
  short sVar1;
  undefined2 *puVar2;
  
  sVar1 = 0x1bf;
  puVar2 = &DAT_00ffcb40;
  do {
    *puVar2 = 0;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  write_word_stream();
  DAT_00ffef14 = DAT_00ffef14 | 0xc;
  return;
}


