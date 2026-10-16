// Ghidra headless export
// Entry: 0x0001582A
// Name: allocate_first_free_effect_slot_local
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


short allocate_first_free_effect_slot_local(void)

{
  short sVar1;
  short sVar2;
  char *pcVar3;
  
  sVar2 = 0x11;
  sVar1 = 0;
  pcVar3 = &DAT_00ffd86c;
  do {
    if (*pcVar3 == '\0') {
      *pcVar3 = '\x01';
      return sVar1;
    }
    sVar1 = sVar1 + 1;
    sVar2 = sVar2 + -1;
    pcVar3 = pcVar3 + 1;
  } while (sVar2 != -1);
  return sVar1;
}


