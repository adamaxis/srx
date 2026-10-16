// Ghidra headless export
// Entry: 0x00019BAE
// Name: select_hud_mode
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined1 select_hud_mode(void)

{
  byte bVar1;
  ushort uVar2;
  
  bVar1 = (&DAT_00ff1113)[(short)(DAT_00fff118 * 0x14)];
  if (((DAT_00fff0a0 < '\x01') || ((char)(bVar1 - 7) < '\0')) || (-1 < (char)(bVar1 - 0x11))) {
    uVar2 = (ushort)bVar1 << 1;
  }
  else {
    uVar2 = (ushort)bVar1 * 2 + 1;
  }
  return *(undefined1 *)((int)&PTR_DAT_000e31a7 + (uint)uVar2);
}


