// Ghidra headless export
// Entry: 0x00012148
// Name: read_entity_action_code
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined1 read_entity_action_code(void)

{
  return *(undefined1 *)
          (DAT_00fffc04 + 0x66 +
          (int)(short)CONCAT31((int3)((uint)DAT_00fff0d1 * 2 >> 8),
                               DAT_00fff0d0 + (char)((uint)DAT_00fff0d1 * 2)));
}


