// Ghidra headless export
// Entry: 0x00004E64
// Name: advance_entity_secondary_timer
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


ushort advance_entity_secondary_timer(void)

{
  char cVar1;
  ushort uVar2;
  int in_A0;
  
  uVar2 = DAT_00ffdf88 & 0x3f;
  if ((DAT_00ffdf88 & 0x3f) != 0) {
    return uVar2;
  }
  cVar1 = *(char *)(in_A0 + 0x41) + -1;
  *(char *)(in_A0 + 0x41) = cVar1;
  if (-1 < cVar1) {
    return uVar2;
  }
  *(byte *)(in_A0 + 0x11) = *(byte *)(in_A0 + 0x11) & 0xdf;
  return uVar2;
}


