// Ghidra headless export
// Entry: 0x00004E38
// Name: FUN_00004e38
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


ushort FUN_00004e38(void)

{
  char cVar1;
  ushort in_D0w;
  ushort uVar2;
  int in_A0;
  
  if ((*(byte *)(in_A0 + 0x9e) & 0x40) == 0) {
    uVar2 = DAT_00ffdf88 & 0x3f;
    if ((DAT_00ffdf88 & 0x3f) != 0) {
      return uVar2;
    }
    cVar1 = *(char *)(in_A0 + 0x40) + -1;
    *(char *)(in_A0 + 0x40) = cVar1;
    if (-1 < cVar1) {
      return uVar2;
    }
    *(byte *)(in_A0 + 0x11) = *(byte *)(in_A0 + 0x11) & 0x7f;
    in_D0w = 0;
  }
  return in_D0w;
}


