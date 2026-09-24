// Ghidra headless export
// Entry: 0x00011624
// Name: FUN_00011624
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00011136 FUN_00011136
//   0x00011678 FUN_00011678
//   0x000116C4 FUN_000116c4


undefined2 FUN_00011624(void)

{
  undefined2 uVar1;
  short sVar2;
  int extraout_A0;
  int iVar3;
  
  uVar1 = FUN_000116c4();
  iVar3 = DAT_00fffc04;
  if ((-1 < *(char *)(DAT_00fffc04 + 0x56)) && (*(char *)(DAT_00fffc04 + 0x56) != '\x7f')) {
    FUN_00011678();
    iVar3 = extraout_A0;
  }
  if (*(char *)(iVar3 + 0x76) != '\0') {
    sVar2 = 7;
    do {
      if ((char)(*(char *)(iVar3 + 0x76) + '+') == *(char *)(iVar3 + 0x66 + (int)sVar2)) {
        FUN_00011678();
        break;
      }
      sVar2 = sVar2 + -1;
    } while (sVar2 != -1);
  }
  FUN_00011136();
  return uVar1;
}


