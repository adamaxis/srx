// Ghidra headless export
// Entry: 0x0001A2CC
// Name: allocate_first_free_effect_slot
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)
// Direct disassembly also seeds the source's secondary outputs before the
// scan: D1 starts at 0x49 and advances by 0x18 per occupied slot, while A2
// starts at 0x0920 and advances by 0x0300. Callers consume those registers
// alongside D0; the typed conversion exposes all three outputs.


short allocate_first_free_effect_slot(void)

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


