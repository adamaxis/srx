// Ghidra headless export
// Entry: 0x00005494
// Name: convert_graphics_routine_0x00005494
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000D44 advance_source_prng
//   0x000013BE build_title_sprite_records_with_attribute_offset
//   0x000061A0 append_entity_sprite_record


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void convert_graphics_routine_0x00005494(void)

{
  byte bVar1;
  ushort uVar2;
  int in_A0;
  int extraout_A0;
  bool bVar3;
  
  bVar1 = *(byte *)(in_A0 + 2) - 1;
  *(byte *)(in_A0 + 2) = bVar1;
  bVar3 = (short)((ushort)bVar1 << 8) < 0;
  if (bVar3) {
    *(undefined1 *)(in_A0 + 2) = 6;
    *(char *)(in_A0 + 3) = *(char *)(in_A0 + 3) + '\x01';
  }
  uVar2 = (ushort)bVar3;
  if ('\x02' < *(char *)(in_A0 + 3)) {
    DAT_00ffe004 = DAT_00ffe004 + -1;
    if (DAT_00ffe004 < 0) {
      *(undefined1 *)(in_A0 + 0xb2) = 0;
      return;
    }
    _DAT_00ffe002 = advance_source_prng();
    DAT_00ffdffe = _DAT_00ffe002 & 7;
    if ((_DAT_00ffe002 & 2) != 0) {
      DAT_00ffdffe = -DAT_00ffdffe;
    }
    uVar2 = _DAT_00ffe002 & 7;
    if ((_DAT_00ffe002 & 4) != 0) {
      uVar2 = -(_DAT_00ffe002 & 7);
    }
    DAT_00ffe000 = uVar2;
    *(undefined1 *)(extraout_A0 + 3) = 0;
    *(undefined1 *)(extraout_A0 + 2) = 5;
  }
  if (uVar2 != 0) {
    append_entity_sprite_record();
  }
  build_title_sprite_records_with_attribute_offset();
  return;
}


