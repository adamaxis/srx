// Ghidra headless export
// Entry: 0x00021A4E
// Name: subtract_resource_clamped
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


void subtract_resource_clamped(void)

{
  int in_D1;
  
  if (in_D1 <= DAT_00fffbfe) {
    DAT_00fffbfe = DAT_00fffbfe - in_D1;
  }
  return;
}


