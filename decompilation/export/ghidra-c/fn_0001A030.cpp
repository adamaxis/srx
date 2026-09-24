// Ghidra headless export
// Entry: 0x0001A030
// Name: FUN_0001a030
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00019BAE FUN_00019bae
//   0x00055FD8 FUN_00055fd8


undefined2 FUN_0001a030(void)

{
  char cVar2;
  undefined2 uVar1;
  undefined1 *unaff_A4;
  undefined1 uVar3;
  bool bVar4;
  
  uVar3 = DAT_00ffdf84 < '\0';
  bVar4 = DAT_00ffdf84 == '\0';
  if ((bVar4) &&
     ((cVar2 = FUN_00019bae(), bVar4 ||
      ((cVar2 != '\x01' && (uVar3 = (char)(cVar2 + -8) < '\0', cVar2 != '\b')))))) {
    FUN_00055fd8();
    if (!(bool)uVar3) {
      uVar1 = func_0x00056050();
      DAT_00fff0aa = *unaff_A4;
      return uVar1;
    }
    FUN_0001b706();
    return 0xffff;
  }
  FUN_00017c84();
  FUN_00017c52();
  FUN_0001b706();
  return 0x45;
}


