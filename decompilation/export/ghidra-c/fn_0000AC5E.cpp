// Ghidra headless export
// Entry: 0x0000AC5E
// Name: clear_entity_effect_if_forced
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0000B7B8 convert_graphics_routine_0x0000b7b8


void clear_entity_effect_if_forced(void)

{
  byte bVar1;
  int in_A0;
  bool bVar2;
  
  if (((*(byte *)(in_A0 + 0x10) & 8) != 0) &&
     (((bVar2 = DAT_00ffdf64 < 0, DAT_00ffdf64 != 0 ||
       (bVar1 = *(byte *)(in_A0 + 0xb0), *(byte *)(in_A0 + 0xb0) = bVar1 & 0xdf, (bVar1 & 0x20) != 0
       )) && (bVar2)))) {
    convert_graphics_routine_0x0000b7b8();
  }
  return;
}


