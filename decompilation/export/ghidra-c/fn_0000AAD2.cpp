// Ghidra headless export
// Entry: 0x0000AAD2
// Name: FUN_0000aad2
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00014226 FUN_00014226


void FUN_0000aad2(void)

{
  if (((*(char *)(DAT_00fffc04 + 0x4e) == '\0') && (*(char *)(DAT_00fffc04 + 0x4f) != '\0')) &&
     (*(short *)(DAT_00fffc04 + 0xdc) < 0)) {
    FUN_00014226();
    return;
  }
  return;
}


