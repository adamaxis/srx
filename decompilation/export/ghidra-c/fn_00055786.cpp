// Ghidra headless export
// Entry: 0x00055786
// Name: apply_dynamic_status_modifier
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000DBA count_thresholded_random_rolls
//   0x0001047E refresh_ready_entity_animations
//   0x00055828 noop_dynamic_modifier_hook


void apply_dynamic_status_modifier(void)

{
  int in_A0;
  int extraout_A0;
  int iVar1;
  int extraout_A0_00;
  
  if ((*(byte *)(in_A0 + 0xdf) & 1) == 0) {
    if ((*(byte *)(in_A0 + 0xdf) & 2) == 0) {
      return;
    }
    noop_dynamic_modifier_hook();
    iVar1 = extraout_A0_00;
  }
  else {
    noop_dynamic_modifier_hook();
    iVar1 = extraout_A0;
  }
  if ((short)(*(short *)(iVar1 + 0x3e) + -3) < 0) {
    refresh_ready_entity_animations();
  }
  count_thresholded_random_rolls();
  return;
}


