// Ghidra headless export
// Entry: 0x0001DC4C
// Name: select_world_entity_service_target
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)

/*
 * A0 is the owner entity and D1 selects whether the source cursor advances.
 * The normal path accepts active, unlocked, unselected, non-high-effect
 * records with a nonzero action field. Modes 0x0B/0x04/0x0C use the source's
 * special active-only path. The native conversion is in
 * ghidra_conversion/entity_service_target_conversion.cpp.
 */

void select_world_entity_service_target(void)
{
  ushort index;
  ushort count;
  bool special_path;
  undefined2 *candidate;

  if (DAT_00fffc04[0x51] != 0) {
    *(byte *)(DAT_00fffc04[0x51] + 0x11) &= 0xef;
    DAT_00fffc04[0x51] = 0;
  }

  special_path = ((char)DAT_00fffc04[0x2b] < 0) &&
                 (DAT_00fffc04[0x57] == 0x0b ||
                  DAT_00fffc04[0x57] == 4 || DAT_00fffc04[0x57] == 0x0c);
  index = source_target_index;
  if (D1 != 0x678cc && source_target_index != 0) {
    ++index;
  }
  count = 0;
  while (count < 8) {
    candidate = entity_service_record(index);
    if ((*(byte *)(candidate + 8) & 1) != 0 &&
        (special_path ||
         ((*(byte *)(candidate + 8) & 0x40) == 0 &&
          (*(byte *)(candidate + 8) & 0x10) == 0 &&
          (*(byte *)(candidate + 0x58) & 0x80) == 0 &&
          candidate[6] != 0))) {
      *(byte *)(candidate + 0x11) |= 0x10;
      DAT_00fffc04[0x51] = candidate;
      candidate[0x53] = 0x8009;
      if (!special_path) {
        candidate[0x56] = DAT_00fffc04;
      }
      return;
    }
    ++index;
    ++count;
  }
}
