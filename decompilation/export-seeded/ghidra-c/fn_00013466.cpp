// Decompiled by Ghidra from Shadowrun Genesis ROM.
// Evidence baseline only: names, types, hardware effects, and data/code boundaries require review.
// ROM address: 0x00013466


void FUN_00013466(void)

{
  char in_D0b;
  undefined1 unaff_D7b;
  int in_A0;
  
  *(char *)(in_A0 + 0x57) = in_D0b;
  if (in_D0b == '\0') {
    *(undefined1 *)(in_A0 + 0x57) = 0;
    *(undefined1 *)(in_A0 + 0x9d) = 0;
    *(undefined1 *)(in_A0 + 0x56) = DAT_0000007f;
  }
  else {
    *(undefined1 *)(in_A0 + 0x56) = unaff_D7b;
    if (*(char *)(in_A0 + 0x4e) != '\0') {
      *(undefined1 *)(in_A0 + 0x4e) = 0;
      *(undefined1 *)(in_A0 + 0x9d) = 0;
    }
  }
  FUN_000036fc();
  FUN_00003404();
  return;
}


