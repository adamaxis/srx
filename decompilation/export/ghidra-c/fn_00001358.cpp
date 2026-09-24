// Ghidra headless export
// Entry: 0x00001358
// Name: copy_source_word_block
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void copy_source_word_block(void)

{
  short sVar1;
  undefined4 *in_A0;
  undefined4 *puVar2;
  
  sVar1 = 0x1f;
  puVar2 = &DAT_00ffd87e;
  do {
    *puVar2 = *in_A0;
    sVar1 = sVar1 + -1;
    in_A0 = in_A0 + 1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  return;
}


