// Ghidra headless export
// Entry: 0x00013CDE
// Name: pass_entity_action_threshold_variant
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00010660 FUN_00010660


void pass_entity_action_threshold_variant(void)

{
  short unaff_D2w;
  int in_A0;
  int extraout_A0;
  
  if (*(char *)(in_A0 + 0x9c) != '\x01') {
    FUN_00010660();
    if ((unaff_D2w < 0) || ((byte)unaff_D2w < *(byte *)(extraout_A0 + 0x9c))) {
      return;
    }
  }
  return;
}


