// Ghidra headless export
// Entry: 0x0000EE86
// Name: replace_nth_terrain_marker
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)
//
// Source behavior: only field mode 2 reaches the marker swap. At least one of
// FFE184, FFE187, or FFFBED==4 must be active; FFE12C must equal FFF0C8;
// FFE180 must be clear. The signed FFE185 ordinal selects a 0xDA marker,
// which becomes 0x9A.


void replace_nth_terrain_marker(void)

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


