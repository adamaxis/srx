// Ghidra headless export
// Entry: 0x000101FE
// Name: select_menu_value
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void select_menu_value(void)

{
  short sVar1;
  
  sVar1 = ((short)CONCAT31((int3)((uint)DAT_00fff0d1 * 7 >> 8),
                           DAT_00fff0d0 + (char)((uint)DAT_00fff0d1 * 7)) + 1) * 2;
  DAT_00fff098 = (ushort)*(byte *)(DAT_00fff10e + (short)(sVar1 + 1));
  if ((*(char *)(DAT_00fffc04 + 0x56) < '\0') &&
     ((char)(*(char *)(DAT_00fff10e + sVar1) + '\x01') == *(char *)(DAT_00fffc04 + 0x57))) {
    DAT_00fff096 = (ushort)*(byte *)(DAT_00fffc04 + 0x9d);
    return;
  }
  DAT_00fff096 = DAT_00fff098;
  return;
}


