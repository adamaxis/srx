// Ghidra headless export
// Entry: 0x0000EE86
// Name: FUN_0000ee86
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void FUN_0000ee86(void)

{
  short sVar1;
  short sVar2;
  char *pcVar3;
  
  if ((DAT_00ffdf40 == 2) &&
     ((((DAT_00ffe184 != '\0' || (DAT_00ffe187 != '\0')) || ((char)DAT_00fffbec == '\x04')) &&
      ((DAT_00ffe12c == DAT_00fff0c8 && (DAT_00ffe180 == '\0')))))) {
    sVar1 = (short)DAT_00ffe185;
    do {
      sVar2 = 0xfff;
      pcVar3 = &DAT_00ff1700;
      do {
        if ((*pcVar3 == -0x26) && (sVar1 = sVar1 + -1, sVar1 < 0)) {
          *pcVar3 = -0x66;
          return;
        }
        sVar2 = sVar2 + -1;
        pcVar3 = pcVar3 + 1;
      } while (sVar2 != -1);
    } while( true );
  }
  return;
}


