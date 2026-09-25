// Ghidra headless export
// Entry: 0x00013228
// Name: FUN_00013228
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x00012148 read_entity_action_code
//   0x00012A54 update_resource_variant


undefined8 FUN_00013228(void)

{
  byte bVar1;
  undefined4 in_D0;
  char cVar2;
  undefined4 in_D1;
  ushort uVar3;
  int extraout_A0;
  int extraout_A0_00;
  int extraout_A0_01;
  int extraout_A0_02;
  int extraout_A0_03;
  int extraout_A0_04;
  int iVar4;
  
  bVar1 = DAT_00fff0d0;
  cVar2 = *(char *)(DAT_00fffc04 + 0x66 + (int)(short)(ushort)DAT_00fff0d0);
  update_resource_variant();
  write_vdp_grid();
  if ((char)(cVar2 + -0xd) < '\0') {
    uVar3 = (ushort)bVar1;
    iVar4 = extraout_A0;
    if ((*(byte *)(extraout_A0 + 0x6e + (int)(short)uVar3) & 2) != 0) {
      cVar2 = write_vdp_grid();
      iVar4 = extraout_A0_00;
    }
    if ((*(byte *)(iVar4 + 0x6e + (int)(short)uVar3) & 4) != 0) {
      cVar2 = write_vdp_grid();
      iVar4 = extraout_A0_01;
    }
    if ((*(byte *)(iVar4 + 0x6e + (int)(short)uVar3) & 1) != 0) {
      cVar2 = write_vdp_grid();
      iVar4 = extraout_A0_02;
    }
    if ((*(byte *)(iVar4 + 0x6e + (int)(short)uVar3) & 8) != 0) {
      cVar2 = write_vdp_grid();
      iVar4 = extraout_A0_03;
    }
    if ((*(byte *)(iVar4 + 0x6e + (int)(short)uVar3) & 0x10) != 0) {
      cVar2 = write_vdp_grid();
      iVar4 = extraout_A0_04;
    }
    if ((*(byte *)(iVar4 + 0x6e + (int)(short)uVar3) & 0x20) != 0) {
      cVar2 = write_vdp_grid();
    }
  }
  if (-1 < (char)(cVar2 + -0xd)) {
    if (((-1 < (char)(cVar2 + -0x14)) && (cVar2 != '\x14')) &&
       (((char)(cVar2 + -0x18) < '\0' || (-1 < (char)(cVar2 + -0x29))))) goto LAB_000133a0;
    read_entity_action_code();
  }
  render_encoded_title_text();
  write_vdp_grid();
LAB_000133a0:
  return CONCAT44(in_D0,in_D1);
}


