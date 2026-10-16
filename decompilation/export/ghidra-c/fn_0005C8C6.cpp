// Ghidra headless export
// Entry: 0x0005C8C6
// Name: upload_title_or_field_word_stream
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


undefined8 upload_title_or_field_word_stream(void)

{
  undefined4 in_D0;
  undefined4 in_D1;
  short extraout_D1w;
  ushort unaff_D2w;
  ushort *extraout_A1;
  
  do {
    make_vdp_transfer_command();
    VDP_DATA = unaff_D2w | *extraout_A1;
  } while (extraout_D1w != 0);
  return CONCAT44(in_D0,in_D1);
}


