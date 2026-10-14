// Ghidra headless export
// Entry: 0x0000C160
// Name: render_wrapped_text
// Decompile status: ok
// Calls recovered from direct call instructions:
//   (none)


/* WARNING: Removing unreachable block (ram,0x0000c18c) */

undefined8 render_wrapped_text(void)

{
  byte bVar1;
  short sVar2;
  undefined4 in_D0;
  undefined4 in_D1;
  short extraout_D1w;
  ushort unaff_D2w;
  short sVar3;
  short sVar4;
  int extraout_A0;
  byte *extraout_A1;
  byte *pbVar5;
  byte *pbVar6;
  
  DAT_00fff0b2 = 1;
  do {
    make_vdp_transfer_command();
    sVar3 = 0;
    pbVar6 = extraout_A1;
    sVar2 = sVar3;
    do {
      sVar4 = sVar2;
      pbVar5 = pbVar6 + 1;
      bVar1 = *pbVar6;
      sVar3 = sVar3 + 1;
      if ((char)bVar1 < '\0') break;
      pbVar6 = pbVar5;
      sVar2 = sVar4;
    } while ((bVar1 != 0x20) || (sVar2 = sVar3, sVar3 < extraout_D1w));
    if (sVar3 <= extraout_D1w) {
      sVar4 = sVar3;
    }
    sVar4 = sVar4 + -1;
    pbVar6 = extraout_A1;
    do {
      bVar1 = *pbVar6;
      if ((char)bVar1 < '\0') {
        if ((char)(bVar1 << 1) < '\0') {
          DAT_00fff0b2 = 0;
          return CONCAT44(in_D0,in_D1);
        }
        break;
      }
      sVar3 = bVar1 - 0x20;
      if (-1 < (char)((char)sVar3 + -0x3d)) {
        sVar3 = bVar1 - 0x24;
      }
      VDP_DATA = unaff_D2w | *(ushort *)(extraout_A0 + (short)(sVar3 << 1)) | 0x8000;
      sVar4 = sVar4 + -1;
      pbVar6 = pbVar6 + 1;
    } while (sVar4 != -1);
  } while( true );
}


