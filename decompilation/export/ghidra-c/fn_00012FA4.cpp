// Ghidra headless export
// Entry: 0x00012FA4
// Name: advance_entity_action_menu
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x00010660 convert_menu_routine_0x00010660


void advance_entity_action_menu(void)

{
  ushort uVar1;
  int extraout_A0;
  char cVar2;
  bool bVar3;
  char cVar4;
  
  render_encoded_title_text();
  uVar1 = (ushort)*(byte *)(extraout_A0 + 0x57);
  cVar4 = SBORROW2(uVar1,1);
  cVar2 = (short)(uVar1 - 1) < 0;
  bVar3 = uVar1 == 1;
  convert_menu_routine_0x00010660();
  if (!bVar3 && cVar4 == cVar2) {
    write_vdp_grid();
  }
  return;
}


