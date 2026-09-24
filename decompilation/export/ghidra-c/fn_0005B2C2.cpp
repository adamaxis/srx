// Ghidra headless export
// Entry: 0x0005B2C2
// Name: set_title_intro_entity_color_words
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void set_title_intro_entity_color_words(void)

{
  short in_D0w;
  undefined2 in_D1w;
  short sVar1;
  undefined2 *puVar2;
  
  sVar1 = 0xe;
  puVar2 = (undefined2 *)((int)&DAT_00ffd87e + (int)(short)(in_D0w * 0x20 + 2));
  do {
    *puVar2 = in_D1w;
    sVar1 = sVar1 + -1;
    puVar2 = puVar2 + 1;
  } while (sVar1 != -1);
  DAT_00ffef14 = DAT_00ffef14 | 1;
  return;
}


