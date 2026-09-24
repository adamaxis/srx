// Ghidra headless export
// Entry: 0x000129FE
// Name: FUN_000129fe
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00003404 FUN_00003404
//   0x00012148 FUN_00012148


void FUN_000129fe(void)

{
  char cVar1;
  short unaff_D5w;
  short unaff_D7w;
  int extraout_A0;
  int extraout_A0_00;
  int iVar2;
  int unaff_A6;
  
  cVar1 = FUN_00012148();
  *(char *)(unaff_A6 + 0x66 + (int)unaff_D5w) = cVar1;
  iVar2 = extraout_A0;
  if ((char)(cVar1 + -0x2c) < '\0') {
    *(undefined1 *)(unaff_A6 + 0x6e + (int)unaff_D5w) =
         *(undefined1 *)(extraout_A0 + 0x6e + (int)unaff_D7w);
    if ((char)unaff_D7w == *(char *)(extraout_A0 + 0x56)) {
      *(undefined1 *)(extraout_A0 + 0x56) = 0x7f;
      *(undefined1 *)(extraout_A0 + 0x57) = 0;
      FUN_00003404();
      iVar2 = extraout_A0_00;
    }
  }
  else {
    *(undefined1 *)(unaff_A6 + 0x6e + (int)unaff_D5w) = 0;
    if (((char)(cVar1 + -0x2b) == *(char *)(extraout_A0 + 0x76)) &&
       (*(char *)(extraout_A0 + 0x6e + (int)unaff_D7w) != '\0')) {
      *(undefined1 *)(extraout_A0 + 0x76) = 0;
    }
  }
  *(undefined1 *)(iVar2 + 0x66 + (int)unaff_D7w) = 0;
  return;
}


