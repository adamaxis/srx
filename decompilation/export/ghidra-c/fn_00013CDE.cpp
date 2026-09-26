// Ghidra headless export
// Entry: 0x00013CDE
// Name: evaluate_entity_animation_rate
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00010660 convert_menu_routine_0x00010660


void evaluate_entity_animation_rate(void)

{
  short unaff_D2w;
  int in_A0;
  int extraout_A0;
  
  if (*(char *)(in_A0 + 0x9c) != '\x01') {
    convert_menu_routine_0x00010660();
    if ((unaff_D2w < 0) || ((byte)unaff_D2w < *(byte *)(extraout_A0 + 0x9c))) {
      return;
    }
  }
  return;
}

