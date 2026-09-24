// Ghidra headless export
// Entry: 0x0000C134
// Name: normalize_source_text_line
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


undefined4 normalize_source_text_line(void)

{
  undefined4 in_D0;
  char cVar1;
  short in_D1w;
  char *in_A1;
  char *unaff_A5;
  
  do {
    cVar1 = *in_A1;
    if (cVar1 < '\0') {
      if ((char)(cVar1 << 1) < '\0') {
        if (-1 < in_D1w) {
          do {
            *unaff_A5 = ' ';
            in_D1w = in_D1w + -1;
            unaff_A5 = unaff_A5 + 1;
          } while (in_D1w != -1);
        }
        return in_D0;
      }
      cVar1 = -0x80;
    }
    *unaff_A5 = cVar1;
    in_A1 = in_A1 + 1;
    unaff_A5 = unaff_A5 + 1;
  } while( true );
}


