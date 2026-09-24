// Ghidra headless export
// Entry: 0x00012A54
// Name: update_resource_variant
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x0001107E upload_resource_stream


undefined4 update_resource_variant(void)

{
  undefined4 in_D0;
  
  if ((short)((short)in_D0 + -0xd) < 0) {
    if (DAT_00fff100 == '\0') {
      return in_D0;
    }
    DAT_00fff100 = 0;
  }
  else {
    if (DAT_00fff100 != '\0') {
      return in_D0;
    }
    DAT_00fff100 = 1;
  }
  upload_resource_stream();
  return in_D0;
}


