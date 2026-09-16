// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x0001275C


/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0001275c(void)

{
  char in_D0b;
  short unaff_D7w;
  int in_A0;
  int extraout_A0;
  
  if ((char)(in_D0b + -0x2c) < '\0') {
    if (_DAT_00fff098 == -1) {
      *(undefined1 *)(in_A0 + 0x56) = 0x7f;
      *(undefined1 *)(in_A0 + 0x57) = 0;
      *(undefined1 *)(in_A0 + 0x4e) = 0;
      FUN_00003404();
      in_A0 = extraout_A0;
    }
  }
  else if (((char)(in_D0b + -0x2b) == *(char *)(in_A0 + 0x76)) &&
          (*(char *)(in_A0 + 0x6e + (int)unaff_D7w) != '\0')) {
    *(undefined1 *)(in_A0 + 0x76) = 0;
  }
  *(undefined1 *)(in_A0 + 0x66 + (int)unaff_D7w) = 0;
  *(undefined1 *)(in_A0 + 0x6e + (int)unaff_D7w) = 0;
  return;
}


