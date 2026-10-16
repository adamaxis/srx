// Ghidra headless export
// Entry: 0x000036FC
// Name: advance_entity_asset_meter
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x00003404 upload_field_asset_setup


undefined8 advance_entity_asset_meter(void)

{
  char *pcVar1;
  char cVar2;
  char cVar3;
  undefined4 in_D0;
  undefined4 in_D1;
  char cVar4;
  ushort uVar5;
  int in_A0;
  
  if ((((*(byte *)(in_A0 + 0x10) & 1) != 0) && (-1 < *(char *)(in_A0 + 0x56))) &&
     (*(char *)(in_A0 + 0x56) != '\x7f')) {
    cVar4 = (char)in_D1;
    if ((char)(*(char *)(in_A0 + 0x57) + -0xd) < '\0') {
      cVar2 = *(char *)(in_A0 + 0x4e);
      cVar3 = cVar4 + cVar2;
      *(char *)(in_A0 + 0x4e) = cVar3;
      if (cVar3 == '\0' || SCARRY1(cVar4,cVar2) != cVar3 < '\0') {
        *(undefined1 *)(in_A0 + 0x4e) = 0;
      }
      else if ((char)((&DAT_000d4564)[(uint)(ushort)(*(byte *)(in_A0 + 0x57) - 1) * 0x30] -
                     *(char *)(in_A0 + 0x4e)) < '\0') {
        *(undefined *)(in_A0 + 0x4e) =
             (&DAT_000d4564)[(uint)(ushort)(*(byte *)(in_A0 + 0x57) - 1) * 0x30];
      }
    }
    else if ((char)(*(char *)(in_A0 + 0x57) + -0x10) < '\0') {
      uVar5 = (ushort)*(byte *)(in_A0 + 0x56);
      pcVar1 = (char *)(in_A0 + 0x6e + (int)(short)uVar5);
      cVar2 = *pcVar1;
      cVar3 = cVar4 + cVar2;
      *pcVar1 = cVar3;
      if (cVar3 == '\0' || SCARRY1(cVar4,cVar2) != cVar3 < '\0') {
        *(undefined1 *)(in_A0 + 0x6e + (int)(short)uVar5) = 0;
        *(undefined1 *)(in_A0 + 0x66 + (int)(short)uVar5) = 0;
        *(undefined1 *)(in_A0 + 0x56) = 0x7f;
        *(undefined1 *)(in_A0 + 0x57) = 0;
        upload_field_asset_setup();
      }
    }
  }
  write_vdp_grid();
  return CONCAT44(in_D0,in_D1);
}


