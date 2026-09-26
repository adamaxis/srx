// Ghidra headless export
// Entry: 0x00013CD4
// Name: evaluate_entity_animation_rate_from_phase
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void evaluate_entity_animation_rate_from_phase(void)

{
  short unaff_D2w;
  int in_A0;
  int extraout_A0;
  
  if (*(char *)(in_A0 + 0x9c) != '\x01') {
    calculate_action_residual();
    if ((unaff_D2w < 0) || ((byte)unaff_D2w < *(byte *)(extraout_A0 + 0x9c))) {
      return;
    }
  }
  return;
}

