// Ghidra headless export
// Entry: 0x0000AF7C
// Name: prepare_entity_delayed_animation_primary
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000ABFC update_entity_animation_delay
//   0x0000AF6C reset_entity_animation_state


void prepare_entity_delayed_animation_primary(void)

{
  ushort uVar1;
  int extraout_A0;
  
  update_entity_animation_delay();
  *(undefined **)(extraout_A0 + 0x3a) = &DAT_0000afe0;
  uVar1 = *(ushort *)(extraout_A0 + 0x2e) << 3 | *(ushort *)(extraout_A0 + 0x2e) >> 0xd;
  if ((short)uVar1 < 0) {
    uVar1 = uVar1 + 1;
  }
  *(undefined2 *)(extraout_A0 + 0x16) = *(undefined2 *)(&DAT_0000c082 + (short)((uVar1 & 7) << 1));
  reset_entity_animation_state();
  return;
}


