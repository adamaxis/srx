// Ghidra headless export
// Entry: 0x00055FD8
// Name: FUN_00055fd8
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00019C54 FUN_00019c54
//   0x00055F44 FUN_00055f44


void FUN_00055fd8(void)

{
  char cVar1;
  short unaff_D6w;
  short sVar2;
  short unaff_D7w;
  char *pcVar3;
  
  DAT_00fff0dd = '\0';
  FUN_00055f44();
  FUN_00019c54();
  if ((short)(unaff_D7w - unaff_D6w) < 0) {
    unaff_D6w = unaff_D7w;
  }
  sVar2 = unaff_D7w + unaff_D6w;
  if (sVar2 == 0) {
    sVar2 = 1;
  }
  if ((char)(*(char *)((int)&DAT_00ff1104 + (short)(DAT_00fff118 * 0x14) + 1) + -1) < (char)sVar2) {
    FUN_000562de();
    count_thresholded_random_rolls();
  }
  cVar1 = DAT_00fff0dd;
  pcVar3 = (char *)(DAT_00fffc04 + 0x9f);
  *pcVar3 = DAT_00fff0dd;
  if (cVar1 < '\x01') {
    *pcVar3 = -1;
  }
  return;
}


