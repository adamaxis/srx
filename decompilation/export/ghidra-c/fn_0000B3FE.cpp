// Ghidra headless export
// Entry: 0x0000B3FE
// Name: set_entity_heading_with_alternate_animation
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000ABFC FUN_0000abfc
//   0x0000AF6C reset_entity_animation_state


void set_entity_heading_with_alternate_animation(void)

{
  ushort in_D0w;
  ushort uVar1;
  int in_A0;
  
  *(ushort *)(in_A0 + 0x2e) = in_D0w;
  *(undefined1 **)(in_A0 + 0x3a) = &LAB_0000b432;
  uVar1 = in_D0w << 3 | in_D0w >> 0xd;
  if ((short)uVar1 < 0) {
    uVar1 = uVar1 + 1;
  }
  *(undefined2 *)(in_A0 + 0x16) =
       *(undefined2 *)((int)&PTR_DAT_0000c092 + (int)(short)((uVar1 & 7) << 1));
  reset_entity_animation_state();
  FUN_0000abfc();
  return;
}


