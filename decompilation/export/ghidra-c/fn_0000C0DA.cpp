// Ghidra headless export
// Entry: 0x0000C0DA
// Name: render_encoded_title_text
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00000E9C make_vdp_transfer_command


undefined8 render_encoded_title_text(void)

{
  byte bVar1;
  undefined4 in_D0;
  short sVar2;
  undefined4 in_D1;
  ushort extraout_D1w;
  int extraout_A0;
  byte *extraout_A1;
  byte *pbVar3;
  
  do {
    make_vdp_transfer_command();
    pbVar3 = extraout_A1;
    while( true ) {
      bVar1 = *pbVar3;
      if ((char)bVar1 < '\0') break;
      sVar2 = bVar1 - 0x20;
      if (-1 < (char)((char)sVar2 + -0x3d)) {
        sVar2 = bVar1 - 0x24;
      }
      VDP_DATA = extraout_D1w | *(ushort *)(extraout_A0 + (short)(sVar2 << 1)) | 0x8000;
      pbVar3 = pbVar3 + 1;
    }
  } while (-1 < (char)(bVar1 << 1));
  return CONCAT44(in_D0,in_D1);
}


