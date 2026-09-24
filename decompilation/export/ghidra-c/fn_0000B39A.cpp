// Ghidra headless export
// Entry: 0x0000B39A
// Name: set_entity_heading_with_field_animation
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000ABFC FUN_0000abfc
//   0x0000AF6C FUN_0000af6c


void set_entity_heading_with_field_animation(void)

{
  ushort in_D0w;
  ushort uVar1;
  int in_A0;
  
  *(ushort *)(in_A0 + 0x2e) = in_D0w;
  *(undefined1 **)(in_A0 + 0x3a) = &LAB_0000b3ee;
  uVar1 = in_D0w << 3 | in_D0w >> 0xd;
  if ((short)uVar1 < 0) {
    uVar1 = uVar1 + 1;
  }
  *(undefined2 *)(in_A0 + 0x16) =
       *(undefined2 *)((int)L"@DHLPTX\\" + (int)(short)((uVar1 & 7) << 1));
  FUN_0000abfc();
  FUN_0000af6c();
  return;
}


