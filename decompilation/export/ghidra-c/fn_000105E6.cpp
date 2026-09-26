// Ghidra headless export
// Entry: 0x000105E6
// Name: refresh_equipment_action_metric
// Decompile status: ok
// Calls recovered from direct call instructions:
//   0x00001232 write_vdp_grid
//   0x0000C0DA render_encoded_title_text
//   0x00010660 calculate_action_residual


void refresh_equipment_action_metric(void)

{
  char cVar1;
  bool bVar2;
  char cVar3;
  
  cVar1 = DAT_00fff096 < 0;
  bVar2 = DAT_00fff096 == 0;
  cVar3 = '\0';
  calculate_action_residual();
  if (bVar2 || cVar3 != cVar1) {
    render_encoded_title_text();
  }
  else {
    write_vdp_grid();
  }
  return;
}


