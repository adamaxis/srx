// Ghidra headless export
// Entry: 0x0001C61C
// Name: convert_world_routine_0x0001c61c
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001378 build_title_sprite_records
//   0x00019CB2 convert_world_routine_0x00019cb2
//   0x0001B612 attach_behavior_descriptor_b


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_world_routine_0x0001c61c(void)

{
  DAT_00ff1594 = DAT_00ff1594 + -1;
  if (DAT_00ff1594 < '\0') {
    DAT_00ff1593 = DAT_00ff1593 + '\x01';
    DAT_00ff1594 = (char)*(undefined2 *)
                          ((int)&PTR_DAT_0001c6d6 + (int)(short)((short)DAT_00ff1593 << 2));
    if ('\x1f' < DAT_00ff1593) {
      DAT_00ff1592 = DAT_00ff1592 & 0xf6;
      attach_behavior_descriptor_b();
      _DAT_00ffe0d6 = 0;
      convert_world_routine_0x00019cb2();
      return;
    }
  }
  if ((DAT_00ff1592 & 8) != 0) {
    return;
  }
  build_title_sprite_records();
  build_title_sprite_records();
  return;
}


